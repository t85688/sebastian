#include "act_southbound.hpp"

#include <QAbstractSocket>
#include <QFile>
#include <QNetworkInterface>
#include <QProcess>
#include <QtEndian>

// #include "act_core.hpp"
#include "act_new_moxa_command_handler.h"
#include "act_restful_client_handler.h"
#include "act_snmp_handler.h"
#include "act_system.hpp"
#include "act_utilities.hpp"

// For Get arp & adapter table
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <iphlpapi.h>
#include <winsock2.h>
#else

#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#endif

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

ACT_STATUS ActSouthbound::InitSnmpResource() {
  ACT_STATUS_INIT();

  ActSnmpHandler snmp_handler;

  return snmp_handler.InitSnmpResource();
}

ACT_STATUS ActSouthbound::ClearSnmpResource() {
  ACT_STATUS_INIT();

  ActSnmpHandler snmp_handler;

  return snmp_handler.ClearSnmpResource();
}

ACT_STATUS ActSouthbound::InitProbeCache() {
  ACT_STATUS_INIT();
  // SNMP
  probe_success_oid_cache_.clear();

  // NETCONF
  probe_yang_cache_.clear();

  return act_status;
}

ACT_STATUS ActSouthbound::PingIpAddress(const QString &ip, const quint8 &times) {
  ACT_STATUS_INIT();

  // Use process to ping
  // ref1: https://www.twblogs.net/a/5eef50361f92b2f1a17d03a2
  // ref2: https://iter01.com/568344.html
  // #Linux "ping -s 1 -c 1 IP"
  // QString cmdstr = QString("ping -s 1 -c 1 %1").arg(ip);

  for (quint8 i = 0; i < times; i++) {
    QProcess proc;
    // windows "ping IP -n 1 -w timeout(ms)"

#ifdef _WIN32
    proc.start("ping", QStringList() << QString(ip) << "-n" << "1" << "-w" << QString::number(ACT_PING_TIMEOUT));

    // proc.waitForFinished(ACT_PING_TIMEOUT);
    // QString response = QString::fromLocal8Bit(proc.readAllStandardOutput());
    // if (response.contains("TTL")) {  // alive
    //   return act_status;
    // }

    if (!proc.waitForFinished(ACT_PING_TIMEOUT)) {
      proc.kill();             // Forcefully terminate if not finished within the timeout
      proc.waitForFinished();  // Ensure the process is terminated
    }

    QString response = QString::fromLocal8Bit(proc.readAllStandardOutput());
    if (response.contains("TTL")) {  // alive
      return act_status;             // Example return value for alive status
    }
#else
    proc.start("ping", QStringList() << QString(ip) << "-w" << QString::number(ACT_PING_TIMEOUT_SECOND) << "-c" << "1");
    proc.waitForFinished(ACT_PING_TIMEOUT);
    QString response = QString::fromLocal8Bit(proc.readAllStandardOutput());
    // qDebug() << __func__ << QString("IP:%1, Response:%2").arg(ip).arg(response).toStdString().c_str();
    if (response.contains("1 received")) {  // alive
      return act_status;
    }

#endif
  }
  return std::make_shared<ActStatusSouthboundFailed>(QString("PING %1 device failed").arg(ip));
}

ACT_STATUS ActSouthbound::ClearArpCache() {
  ACT_STATUS_INIT();

  // Use process to clear arp cache
  QProcess proc;

#ifdef _WIN32
  // windows "arp -d"
  proc.start("arp", QStringList() << "-d");
  proc.waitForFinished(-1);  // wait for the process to finish
  return act_status;
#else
  // linux "ip neigh flush all"
  proc.start("ip", QStringList() << "neigh" << "flush" << "all");
  proc.waitForFinished(-1);  // wait for the process to finish
  return act_status;
#endif

  return std::make_shared<ActStatusSouthboundFailed>(QString("Clear ARP cache failed"));
}

ACT_STATUS ActSouthbound::UpdateDeviceIcmpStatusThread(ActDevice device, QList<ActDevice> &result_update_icmp_devices) {
  ACT_STATUS_INIT();

  bool connect_status = false;

  act_status = PingIpAddress(device.GetIpv4().GetIpAddress(), ACT_PING_REPEAT_TIMES);
  if (IsActStatusSuccess(act_status)) {  // alive
    connect_status = true;
  }

  // Set device status
  auto dev_status = device.GetDeviceStatus();
  dev_status.SetICMPStatus(connect_status);
  device.SetDeviceStatus(dev_status);

  // Add device to result
  mutex_.lock();
  result_update_icmp_devices.append(device);
  mutex_.unlock();

  return act_status;
}

ACT_STATUS ActSouthbound::UpdateDevicesIcmpStatus(QList<ActDevice> &devices) {
  ACT_STATUS_INIT();
  QList<ActDevice> result_update_icmp_devices;

  // Use multiple threads to ping device
  quint32 thread_count = 0;
  quint32 max_thread = 10;
  std::unique_ptr<std::thread[]> threads = std::make_unique<std::thread[]>(max_thread);

  for (auto device : devices) {
    if (stop_flag_) {
      return ACT_STATUS_STOP;
    }

    threads[thread_count] =
        std::thread(&ActSouthbound::UpdateDeviceIcmpStatusThread, this, device, std::ref(result_update_icmp_devices));
    thread_count++;

    if (thread_count >= max_thread) {
      // Wait thread done and remove delete it
      for (quint32 i = 0; i < thread_count; i++) {
        if (threads[i].joinable()) {
          threads[i].join();
        }
      }
      thread_count = 0;
    }
  }

  if (thread_count != 0) {
    for (quint32 i = 0; i < thread_count; i++) {
      if (threads[i].joinable()) {
        threads[i].join();
      }
    }
  }

  // Update device's SNMP status (keep origin sequence)
  for (auto &device : devices) {
    // Get southbound update device in result_update_icmp_devices
    auto south_device_index = result_update_icmp_devices.indexOf(device);
    if (south_device_index == -1) {
      qCritical() << __func__ << "Southbound device not found. DeviceID:" << device.GetId();
      return std::make_shared<ActStatusNotFound>(QString("Southbound Device(%1)").arg(device.GetId()));
    }
    auto south_device = result_update_icmp_devices.at(south_device_index);

    device.GetDeviceStatus().GetICMPStatus() = south_device.GetDeviceStatus().GetICMPStatus();
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ScanDevicesByScanIpRange(const QList<ActScanIpRangeEntry> &scan_ip_ranges,
                                                   QSet<ActDevice> &result_devices) {
  ACT_STATUS_INIT();
  result_devices.clear();

  QList<ActDevice> device_list;
  QSet<quint32> keep_dev_ip_num;

  for (auto scan_ip_range_entry : scan_ip_ranges) {
    // Get IP num
    quint32 ip_num, end_ip_num;
    ActIpv4::AddressStrToNumber(scan_ip_range_entry.GetStartIp(), ip_num);
    ActIpv4::AddressStrToNumber(scan_ip_range_entry.GetEndIp(), end_ip_num);

    // Generate device
    while (ip_num <= end_ip_num) {
      // Skip duplicate ip_num (when scan_ip_range has overlap)
      if (keep_dev_ip_num.contains(ip_num)) {
        ip_num++;
        continue;
      }
      keep_dev_ip_num.insert(ip_num);

      QString ip_str;
      ActIpv4::AddressNumberToStr(ip_num, ip_str);  // get ip string
      ActDevice device(ip_str);
      device.SetId(ip_num);
      ip_num++;

      device.SetAccount(scan_ip_range_entry.GetAccount());
      device.SetNetconfConfiguration(scan_ip_range_entry.GetNetconfConfiguration());
      device.SetSnmpConfiguration(scan_ip_range_entry.GetSnmpConfiguration());
      device.SetRestfulConfiguration(scan_ip_range_entry.GetRestfulConfiguration());

      // Set EnableSnmpSetting flag
      // [feat:2391] Save Enable SNMP setting in device configuration
      device.SetEnableSnmpSetting(scan_ip_range_entry.GetEnableSnmpSetting());

      // Assign AutoProbe flag
      // [feat:2604] Auto Scan - Get the unknown device need to execute the auto probe
      device.auto_probe_ = scan_ip_range_entry.GetAutoProbe();

      device_list.append(device);

      // Batch update devices ICMP status every 200 devices
      if (device_list.size() >= 200) {
        act_status = UpdateDevicesIcmpStatus(device_list);
        if (stop_flag_) {
          return ACT_STATUS_STOP;
        }
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << __func__ << "UpdateDevicesIcmpStatus() failed.";
          return act_status;
        }

        // Get alive device
        for (auto new_device : device_list) {
          if (new_device.GetDeviceStatus().GetICMPStatus()) {
            result_devices.insert(new_device);
          }
        }
        device_list.clear();
      }
    }
  }

  // Batch update devices ICMP status at last
  if (!device_list.isEmpty()) {
    act_status = UpdateDevicesIcmpStatus(device_list);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "UpdateDevicesIcmpStatus() failed.";
      return act_status;
    }

    // Get alive device
    for (auto new_device : device_list) {
      if (new_device.GetDeviceStatus().GetICMPStatus()) {
        result_devices.insert(new_device);
      }
    }
  }
  return act_status;
}

ACT_STATUS ActSouthbound::CheckMethodProtocolsStatus(const ActDevice &device, const ActFeatureMethod &feat_method) {
  ACT_STATUS_INIT();

  for (auto protocol : feat_method.GetProtocols().keys()) {
    if (protocol == snmp_str_) {  // SNMP
      if (!device.GetDeviceStatus().GetSNMPStatus()) {
        return DeviceConnectionFalseErrorHandler(__func__, device, ActConnectProtocolTypeEnum::kSNMP);
      }
    } else if (protocol == netconf_str_) {  // NETCONF
      if (!device.GetDeviceStatus().GetNETCONFStatus()) {
        return DeviceConnectionFalseErrorHandler(__func__, device, ActConnectProtocolTypeEnum::kNETCONF);
      }
    } else if (protocol == moxa_command_str_) {  // NewMOXAcommand
      if (!device.GetDeviceStatus().GetNewMOXACommandStatus()) {
        return DeviceConnectionFalseErrorHandler(__func__, device, ActConnectProtocolTypeEnum::kMOXAcommand);
      }
    } else {  // RESTful
      if (!device.GetDeviceStatus().GetRESTfulStatus()) {
        return DeviceConnectionFalseErrorHandler(__func__, device, ActConnectProtocolTypeEnum::kRESTful);
      }
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::FindDeviceByLldpChassisId(const QList<ActDevice> &devices, const qint64 &chassis_id_subtype,
                                                    const QString &chassis_id, ActDevice &result_device) {
  ACT_STATUS_INIT();
  if (devices.isEmpty()) {
    QString error_msg = QString("The devices list is empty");
    // qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (chassis_id.isEmpty()) {
    QString error_msg = QString("The chassis_id is empty");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // * First step: Use chassis_id to mapping Remote Device's lldp_chassis_id
  for (auto device : devices) {
    // Skip device.lldp_chassis_id not empty
    if (device.lldp_data_.GetLocChassisId().isEmpty()) {
      continue;
    }
    // Mapping
    if (device.lldp_data_.GetLocChassisId() == chassis_id) {
      result_device = device;
      return ACT_STATUS_SUCCESS;
    }
  }

  // * Second step: If the first step not found (Optional)
  //  - Use the subtype to find the corresponding fields on the Remote Device
  //  - Because the Remote device can’t access the LLDP local Chassis ID

  // Chassis component(1) || MAC address(4) || Locally assigned(7)
  if (chassis_id_subtype == 1 || chassis_id_subtype == 4 || chassis_id_subtype == 7) {
    // Try to find device by MAC
    QString mac_address = "";
    auto transfer_status = TransferChassisIdToMacFormat(chassis_id, mac_address);
    if (IsActStatusSuccess(transfer_status)) {
      // Find Device
      for (auto device : devices) {
        if (device.GetMacAddress() == mac_address) {
          result_device = device;
          return ACT_STATUS_SUCCESS;
        }
      }
    }
  }

  // Chassis component(1) || Network address(5) || Locally assigned(7)
  if (chassis_id_subtype == 1 || chassis_id_subtype == 5 || chassis_id_subtype == 7) {
    // Try to find device Ipv4 address
    for (auto device : devices) {
      if (device.GetIpv4().GetIpAddress() == chassis_id) {
        result_device = device;
        return ACT_STATUS_SUCCESS;
      }
    }
  }

  // Skip. Device hasn't Chassis Component field.
  // Chassis component(1)
  // if (chassis_id_subtype == 1) {
  //   // Try to find device component
  // }

  // Skip. To avoid duplicate interface name in the multiple devices.
  // // Interface alias(2) || Port component(3) || Interface name(6)
  // if (chassis_id_subtype == 2) {
  //   // Try to find device by interface name
  // }

  // Return NotFound status
  QString not_found_elem = QString("Remote device (by ChassisID: %1(%2))").arg(chassis_id).arg(chassis_id_subtype);
  // qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
  return std::make_shared<ActStatusNotFound>(not_found_elem);
}

ACT_STATUS ActSouthbound::FindAndUpdateRemoteDeviceInterface(const ActDevice &rem_dev,
                                                             QSet<ActDevice> &update_device_set,
                                                             qint64 &result_rem_interface_id) {
  ACT_STATUS_INIT();
  qint64 rem_interface_id = -1;
  // Modify dev interface's InterfaceName
  ActDevice new_rem_dev = rem_dev;

  // If update_device has this device would use it
  auto update_dev_it = update_device_set.find(new_rem_dev);
  if (update_dev_it != update_device_set.end()) {  // found
    new_rem_dev = *update_dev_it;
  }

  // Update the interface if that has the same interface_name in the interface_list
  for (auto &dev_interface : new_rem_dev.GetInterfaces()) {
    if (dev_interface.GetInterfaceName() == ACT_AUTOSCAN_ASSIGN_INTERFACE_NAME) {
      // Same InterfaceName, update it (MAC address & DeviceId)
      dev_interface.SetDeviceId(rem_dev.GetId());
      dev_interface.SetIpAddress(rem_dev.GetIpv4().GetIpAddress());
      dev_interface.SetUsed(true);
      ActLink init_link;
      dev_interface.SetSupportSpeeds({init_link.GetSpeed()});
      // Get the rem_interface_id
      rem_interface_id = dev_interface.GetInterfaceId();
    }
  }
  // When not found the interface same as the interface_name.
  // Append the new_interface to last.
  if (rem_interface_id == -1) {
    qint64 if_id = new_rem_dev.GetInterfaces().size() + 1;
    ActInterface new_interface(if_id);
    new_interface.SetDeviceId(rem_dev.GetId());
    new_interface.SetInterfaceName(ACT_AUTOSCAN_ASSIGN_INTERFACE_NAME);
    new_interface.SetIpAddress(rem_dev.GetIpv4().GetIpAddress());
    new_interface.SetUsed(true);
    ActLink init_link;
    new_interface.SetSupportSpeeds({init_link.GetSpeed()});
    new_rem_dev.GetInterfaces().append(new_interface);
    rem_interface_id = if_id;
  }

  // Update update_device_set
  if (update_dev_it != update_device_set.end()) {
    update_device_set.remove(new_rem_dev);
  }
  update_device_set.insert(new_rem_dev);
  result_rem_interface_id = rem_interface_id;

  // TODO: remove
  // [feat:3661] Remove fake MAC
  // //  Not found remote interface at update_device_set, would set interface name to "Port" & update device
  // if (rem_interface_id == -1) {
  //   // Update the interface if that has the same interface_name in the interface_list
  //   for (auto &dev_interface : new_rem_dev.GetInterfaces()) {
  //     if (dev_interface.GetInterfaceName() == ACT_AUTOSCAN_ASSIGN_INTERFACE_NAME) {
  //       // Same InterfaceName, update it (MAC address & DeviceId)
  //       dev_interface.SetDeviceId(rem_dev.GetId());
  //       dev_interface.SetMacAddress(rem_dev.GetMacAddress());
  //       dev_interface.SetIpAddress(rem_dev.GetIpv4().GetIpAddress());
  //       dev_interface.SetMacAddress(rem_dev.GetMacAddress());
  //       dev_interface.SetUsed(true);
  //       ActLink init_link;
  //       dev_interface.SetSupportSpeeds({init_link.GetSpeed()});
  //       // Get the rem_interface_id
  //       rem_interface_id = dev_interface.GetInterfaceId();
  //     }
  //   }

  //   // When not found the interface same as the interface_name.
  //   // Append the new_interface to last.
  //   if (rem_interface_id == -1) {
  //     qint64 if_id = new_rem_dev.GetInterfaces().size() + 1;
  //     ActInterface new_interface(if_id);
  //     new_interface.SetDeviceId(rem_dev.GetId());
  //     new_interface.SetInterfaceName(ACT_AUTOSCAN_ASSIGN_INTERFACE_NAME);
  //     new_interface.SetIpAddress(rem_dev.GetIpv4().GetIpAddress());
  //     new_interface.SetMacAddress(rem_dev.GetMacAddress());
  //     new_interface.SetUsed(true);
  //     ActLink init_link;
  //     new_interface.SetSupportSpeeds({init_link.GetSpeed()});
  //     new_rem_dev.GetInterfaces().append(new_interface);
  //     rem_interface_id = if_id;
  //   }

  //   // Update update_device_set
  //   if (update_dev_it != update_device_set.end()) {
  //     update_device_set.remove(new_rem_dev);
  //   }
  //   update_device_set.insert(new_rem_dev);
  // }

  // result_rem_interface_id = rem_interface_id;
  return ACT_STATUS_SUCCESS;
}
ACT_STATUS ActSouthbound::GenerateLinkSetBylldpInfo(const ActDevice &device, const QList<ActDevice> &alive_devices,
                                                    ActScanLinksResult &scan_link_result) {
  ACT_STATUS_INIT();
  QSet<ActLink> link_set;
  QSet<ActDevice> update_device_set;
  auto loc_lldp_data = device.lldp_data_;

  for (auto loc_interface_id : loc_lldp_data.GetRemPortChassisIdMap().keys()) {
    QString rem_chassis_id = loc_lldp_data.GetRemPortChassisIdMap()[loc_interface_id];
    qint64 rem_chassis_id_subtype = -1;
    qint64 rem_interface_id = -1;
    qint64 rem_dev_id = -1;

    // Find rem_chassis_id_subtype entry
    if (!loc_lldp_data.GetRemPortChassisIdSubtypeMap().contains(loc_interface_id)) {
      // qDebug() << __func__
      //          << QString("Can not find LldpRemChassisIDSubtype(%1). Device:%1")
      //                 .arg(loc_interface_id)
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }
    rem_chassis_id_subtype = loc_lldp_data.GetRemPortChassisIdSubtypeMap()[loc_interface_id];

    // Get remote device
    ActDevice rem_dev;
    auto other_devices = alive_devices;
    other_devices.removeOne(device);  // skip find the local device
    auto find_status = FindDeviceByLldpChassisId(other_devices, rem_chassis_id_subtype, rem_chassis_id, rem_dev);
    if (!IsActStatusSuccess(find_status) || (rem_dev.GetId() == -1)) {  // failed
      // qDebug() << __func__
      //          << QString("Device(%1) LLDP RemChassisId(%2(%3)) not found Remote Device")
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .arg(rem_chassis_id)
      //                 .arg(rem_chassis_id_subtype)
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    // Check Source & Destination is different
    if (device.GetId() == rem_dev.GetId()) {
      // qDebug() << __func__
      //          << QString("Device(%1) skip the link that with the same Source & Destination device ID.")
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    rem_dev_id = rem_dev.GetId();

    // Find rem_port_id entry
    auto rem_port_id_map_it = loc_lldp_data.GetRemPortIdMap().find(loc_interface_id);
    if (rem_port_id_map_it == loc_lldp_data.GetRemPortIdMap().end()) {
      // qDebug() << __func__
      //          << QString("Can not find LldpRemPortId(%1). Device:%1")
      //                 .arg(loc_interface_id)
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    // Find remote device local lldp_port_id
    auto rem_dev_loc_port_id_map = rem_dev.lldp_data_.GetLocPortIdMap();
    for (auto rem_port_key : rem_dev_loc_port_id_map.keys()) {
      if (rem_dev_loc_port_id_map[rem_port_key] == rem_port_id_map_it.value()) {
        rem_interface_id = rem_port_key;
        break;
      }
    }

    // If not found would try to use directly transfer to interface_id
    if (rem_interface_id == -1) {
      rem_interface_id = rem_port_id_map_it.value().toInt();
    }

    // If remote device can't access the lldp_port_id
    if (rem_interface_id == -1 || rem_interface_id == 0) {
      // Get rem_interface_id
      auto rem_port_id_subtype_map_it = loc_lldp_data.GetRemPortIdSubtypeMap().find(loc_interface_id);
      if (rem_port_id_subtype_map_it == loc_lldp_data.GetRemPortIdSubtypeMap().end()) {
        // qDebug() << __func__
        //          << QString("Can not find LldpRemPortIdSubtype(%1). Device:%1")
        //                 .arg(loc_interface_id)
        //                 .arg(device.GetIpv4().GetIpAddress())
        //                 .toStdString()
        //                 .c_str();
        continue;
      }

      // Find rem device interface names
      // 1:InterfaceAlias,  7: local(moxa assign port_index)
      if (rem_port_id_subtype_map_it.value() == 1 || rem_port_id_subtype_map_it.value() == 7) {
        bool ok;
        rem_interface_id = rem_port_id_map_it.value().toInt(&ok, 10);
        if (!ok || (rem_interface_id == 0)) {
          for (auto dev_interface : rem_dev.GetInterfaces()) {
            // qDebug() << __func__ << "if_name:" << dev_interface.GetInterfaceName()
            //          << "target_if:" << rem_port_id_map_it.value();
            if (dev_interface.GetInterfaceName() == rem_port_id_map_it.value()) {
              rem_interface_id = dev_interface.GetInterfaceId();
              break;
            }
          }
        }

        // TODO: remove
        // [feat:3661] Remove fake MAC
        // } else if (rem_port_id_subtype_map_it.value() == 3) {
        //   // Find rem device interface MacAddress
        //   // 3:MacAddress
        //   for (auto dev_interface : rem_dev.GetInterfaces()) {
        //     // qDebug() << __func__ << "if_mac:" << dev_interface.GetMacAddress()
        //     //          << "target_if_mac:" << rem_port_id_map_it.value();
        //     if (dev_interface.GetMacAddress() == rem_port_id_map_it.value()) {
        //       rem_interface_id = dev_interface.GetInterfaceId();
        //       break;
        //     }
        //   }
      } else if (rem_port_id_subtype_map_it.value() == 5) {
        // Find rem device interface name
        // 5:InterfaceName
        for (auto dev_interface : rem_dev.GetInterfaces()) {
          // qDebug() << __func__ << "if_name:" << dev_interface.GetInterfaceName()
          //          << "target_if:" << rem_port_id_map_it.value();
          if (dev_interface.GetInterfaceName() == rem_port_id_map_it.value()) {
            rem_interface_id = dev_interface.GetInterfaceId();
            break;
          }
        }
      }
    }

    // Not found rem interface, would set interface name to "Port" amd update device
    if (rem_interface_id == -1 || rem_interface_id == 0) {
      FindAndUpdateRemoteDeviceInterface(rem_dev, update_device_set, rem_interface_id);
    }

    // Not found rem interface
    if (rem_interface_id == -1 || rem_interface_id == 0) {
      // qDebug() << __func__
      //          << QString("Device(%1) LLDP RemPortId(%2) not found at RemoteDevice(%3)")
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .arg(rem_port_id_map_it.value())
      //                 .arg(rem_dev.GetIpv4().GetIpAddress())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    // Create ActLink
    qint64 link_id = link_set.size() + 1;
    ActLink link(link_id, device.GetId(), rem_dev.GetId(), loc_interface_id, rem_interface_id);
    link_set.insert(link);
  }

  // // Create the result port_mac_map
  // QMap<qint64, QString> port_mac_map;
  // for (auto port : loc_lldp_data.GetRemPortChassisIdMap().keys()) {
  //   // Try to transfer to MAC, not all chassis_id is the MAC
  //   QString mac_address = "";
  //   auto transfer_status = TransferChassisIdToMacFormat(loc_lldp_data.GetRemPortChassisIdMap()[port], mac_address);
  //   if (!IsActStatusSuccess(transfer_status)) {
  //     continue;
  //   }
  //   port_mac_map[port] = mac_address;
  // }

  // Update result
  scan_link_result.SetUpdateDevices(update_device_set);
  scan_link_result.SetScanLinks(link_set);
  // scan_link_result.SetPortMacMap(port_mac_map);

  return act_status;
}

ACT_STATUS ActSouthbound::GenerateLinkSetByMacTable(const ActDevice &device, const QList<ActDevice> &alive_devices,
                                                    const QSet<ActLink> &exist_links,
                                                    const QMap<QString, QString> &ip_mac_table,
                                                    ActScanLinksResult &scan_link_result) {
  ACT_STATUS_INIT();
  QSet<ActLink> link_set;
  QSet<ActDevice> update_device_set;

  qint64 link_id = 1;

  auto port_mac_map = device.single_entry_mac_table_;
  for (auto port_id : port_mac_map.keys()) {
    // Check MAC address not empty
    if (port_mac_map[port_id].isEmpty()) {
      continue;
    }

    // Create Link
    // Get new link id
    while (exist_links.contains(ActLink(link_id))) {
      link_id += 1;
    }
    ActLink link(link_id);

    // Set link src
    link.SetSourceDeviceId(device.GetId());
    link.SetSourceInterfaceId(port_id);
    auto link_it = exist_links.find(link);
    if (link_it != exist_links.end()) {  // link already exist
      // qDebug() << __func__ << "The link already exists";
      continue;
    }

    // Find remote dev id
    qint64 rem_dev_id;
    QString rem_dev_ip = "";
    qint64 rem_interface_id = -1;
    QString rem_interface_mac = port_mac_map[port_id];

    // Check MAC address format
    if (!IsActStatusSuccess(CheckMacAddress(rem_interface_mac))) {
      // qDebug() << __func__
      //          << QString("Device(%1) skip invalid MAC(%2)")
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .arg(rem_interface_mac)
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    // Get ip by mac
    rem_dev_ip = ip_mac_table.key(rem_interface_mac);
    if (rem_dev_ip == "") {  // not found ip
      // qDebug() << __func__
      //          << QString("Device(%1) skip the rem_dev_ip is empty")
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .toStdString()
      //                 .c_str();

      continue;
    }

    // Skip same device IP
    // Check Source & Destination is different
    if (rem_dev_ip == device.GetIpv4().GetIpAddress()) {
      // qDebug() << __func__
      //          << QString("Device(%1) skip the rem_dev_ip is same as device IP")
      //                 .arg(device.GetIpv4().GetIpAddress())
      //                 .toStdString()
      //                 .c_str();
      continue;
    }

    // Transfer the rem_dev_ip to rem_dev_id
    quint32 ip_num;
    ActIpv4::AddressStrToNumber(rem_dev_ip, ip_num);
    rem_dev_id = ip_num;
    // Find remote device to check it is alive device.
    auto rem_dev_idx = alive_devices.indexOf(ActDevice(rem_dev_id));
    if (rem_dev_idx == -1) {  // not alive device
      // qDebug()
      //     << __func__
      //     << QString("Device(%1) the rem_dev is not
      //     alive").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();

      continue;
    }

    // Modify dev interface's InterfaceName & MacAddress
    auto new_rem_dev = alive_devices.at(rem_dev_idx);
    FindAndUpdateRemoteDeviceInterface(new_rem_dev, update_device_set, rem_interface_id);

    // Set link dst
    link.SetDestinationDeviceId(rem_dev_id);
    link.SetDestinationInterfaceId(rem_interface_id);  // to be dealt with all links.
    link_set.insert(link);
  }

  // Update result
  scan_link_result.SetUpdateDevices(update_device_set);
  scan_link_result.SetScanLinks(link_set);

  return act_status;
}

ACT_STATUS ActSouthbound::FromVlanStaticToVlanPortType(const ActDevice &device,
                                                       const QSet<ActVlanStaticEntry> &vlan_static_entries,
                                                       QSet<ActVlanPortTypeEntry> &vlan_port_type_entries) {
  ACT_STATUS_INIT();
  vlan_port_type_entries.clear();

  for (auto entry : vlan_static_entries) {
    // Egress port
    for (auto port : entry.GetEgressPorts()) {
      // [bugfix:881] Deploy sequence should go from far to near
      if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
        vlan_port_type_entries.insert(
            ActVlanPortTypeEntry(port, ActVlanPortTypeEnum::kHybrid, ActVlanPriorityEnum::kNonTSN));  // hybrid
      } else {
        vlan_port_type_entries.insert(
            ActVlanPortTypeEntry(port, ActVlanPortTypeEnum::kTrunk, ActVlanPriorityEnum::kNonTSN));  // trunk
      }
    }

    // Untag port (only hybrid)
    // [feat: 592] untag for hybrid
    for (auto port : entry.GetUntaggedPorts()) {
      // [bugfix:881] Deploy sequence should go from far to near
      if (device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
        vlan_port_type_entries.insert(
            ActVlanPortTypeEntry(port, ActVlanPortTypeEnum::kHybrid, ActVlanPriorityEnum::kNonTSN));  // hybrid
      }
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::GetIpMacTable(QMap<QString, QString> &ip_mac_map_result) {
  ACT_STATUS_INIT();
  ip_mac_map_result.clear();

  // LocalhostArpTable & LocalhostAdapterTable

  QMap<QString, QString> arp_table, adapter_table;

  // Get Localhost ARP table
  QSet<QString> arp_entry_types;
  arp_entry_types << ACT_ARP_ENTRY_DYNAMIC  // dynamic
                  << ACT_ARP_ENTRY_STATIC;  // static
  act_status = GetLocalhostArpTable(arp_entry_types, arp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostArpTable() failed.";
    return act_status;
  }

  // Get Localhost Adapter Table
  act_status = GetLocalhostAdapterTable(adapter_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostAdapterTable() failed.";
    return act_status;
  }

  // merge two table
  foreach (auto ip, adapter_table.keys()) {
    arp_table.insert(ip, adapter_table[ip]);
  }

  ip_mac_map_result = arp_table;

  // foreach (auto ip, ip_mac_map_result.keys()) {
  //   qDebug() << QString("%1 - %2").arg(ip, -15).arg(ip_mac_map_result[ip]).toStdString().c_str();
  // }
  return act_status;
}

ACT_STATUS ActSouthbound::GetLocalhostAdapterIpDeviceTable(QMap<QString, QString> &ip_dev_map_result) {
  ACT_STATUS_INIT();
  ip_dev_map_result.clear();
  try {
    // QNetworkInterface
    // ref: https://www.cnblogs.com/liushui-sky/p/6479110.html
    // ref:
    // https://stackoverflow.com/questions/5572263/get-current-qnetworkinterface-active-and-connected-to-the-internet
    const QList<QNetworkInterface> netinterfaces = QNetworkInterface::allInterfaces();

    for (auto netinterface : netinterfaces) {
      if (!netinterface.isValid()) {  // check valid
        continue;
      }

      QNetworkInterface::InterfaceFlags flags = netinterface.flags();
      if ((!flags.testFlag(QNetworkInterface::IsRunning)) ||
          flags.testFlag(QNetworkInterface::IsLoopBack)) {  // check flags(IsRunning & not LoopBack)
        continue;
      }

      if (netinterface.type() != QNetworkInterface::Ethernet) {  // check Ethernet type
        continue;
      }

      for (auto entry : netinterface.addressEntries()) {
        if (entry.ip() == QHostAddress::LocalHost) {  // check not local host
          continue;
        }

        if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) {  // check is ipv4 addresses
          continue;
        }
        // Insert entry
        QString ip_str = entry.ip().toString();
        ip_dev_map_result.insert(ip_str, netinterface.name());
      }
    }
  } catch (std::exception &e) {
    qCritical() << __func__ << "Get HostMacAddress failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }
  return act_status;
}

ACT_STATUS ActSouthbound::GetLocalhostAdapterTable(QMap<QString, QString> &ip_mac_map_result) {
  ACT_STATUS_INIT();
  ip_mac_map_result.clear();
  try {
    // QNetworkInterface
    // ref: https://www.cnblogs.com/liushui-sky/p/6479110.html
    // ref:
    // https://stackoverflow.com/questions/5572263/get-current-qnetworkinterface-active-and-connected-to-the-internet
    const QList<QNetworkInterface> netinterfaces = QNetworkInterface::allInterfaces();

    for (auto netinterface : netinterfaces) {
      if (!netinterface.isValid()) {  // check valid
        continue;
      }
      QNetworkInterface::InterfaceFlags flags = netinterface.flags();
      if (flags.testFlag(QNetworkInterface::IsLoopBack)) {  // check flags(not LoopBack)
        continue;
      }
      if (netinterface.type() != QNetworkInterface::Ethernet) {  // check Ethernet type
        continue;
      }

      QString mac_str = netinterface.hardwareAddress();
      mac_str.replace(":", "-");    // 00:15:5d:37:eb:0e -> 00-15-5d-37-eb-0e
      mac_str = mac_str.toUpper();  // 00-15-5d-37-eb-0e -> 00-15-5D-37-EB-0E

      if (!IsActStatusSuccess(CheckMacAddress(mac_str))) {
        continue;
      }

      for (auto entry : netinterface.addressEntries()) {
        if (entry.ip() == QHostAddress::LocalHost) {  // check not local host
          continue;
        }

        if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) {  // check is ipv4 addresses
          continue;
        }
        // Insert entry
        QString ip_str = entry.ip().toString();
        ip_mac_map_result.insert(ip_str, mac_str);
      }
    }
  } catch (std::exception &e) {
    qCritical() << __func__ << "Get HostMacAddress failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }
  return act_status;
}

ACT_STATUS ActSouthbound::GetLocalhostArpTable(const QSet<QString> &arp_entry_types,
                                               QMap<QString, QString> &ip_mac_map_result) {
  ACT_STATUS_INIT();
  ip_mac_map_result.clear();
  // Dynamic(3), Static(4)

#ifdef _WIN32
  try {
    DWORD i;
    PMIB_IPNETTABLE pIpNetTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    DWORD dwResult;

    dwResult = GetIpNetTable(NULL, &dwSize, 0);
    if (dwResult == ERROR_INSUFFICIENT_BUFFER) {
      // qDebug() << __func__ << "ERROR_INSUFFICIENT_BUFFER";
      pIpNetTable = (MIB_IPNETTABLE *)malloc(dwSize);
    }

    dwRetVal = GetIpNetTable(pIpNetTable, &dwSize, 0);
    if (dwRetVal == NO_ERROR) {
      if (pIpNetTable->dwNumEntries > 0) {
        for (i = 0; i < pIpNetTable->dwNumEntries; i++) {
          // Get arp entry type
          // Other(1), Invalidated(2), Dynamic(3), Static(4)
          QString type_str = QString("%1").arg(pIpNetTable->table[i].dwType);
          if (!arp_entry_types.contains(type_str)) {
            continue;
          }
          // if (type_str == "2") {
          //   continue;
          // }

          // Get MacAdddress
          QString mac_str = QString("%1-%2-%3-%4-%5-%6")
                                .arg(pIpNetTable->table[i].bPhysAddr[0], 2, 16, QLatin1Char('0'))
                                .arg(pIpNetTable->table[i].bPhysAddr[1], 2, 16, QLatin1Char('0'))
                                .arg(pIpNetTable->table[i].bPhysAddr[2], 2, 16, QLatin1Char('0'))
                                .arg(pIpNetTable->table[i].bPhysAddr[3], 2, 16, QLatin1Char('0'))
                                .arg(pIpNetTable->table[i].bPhysAddr[4], 2, 16, QLatin1Char('0'))
                                .arg(pIpNetTable->table[i].bPhysAddr[5], 2, 16, QLatin1Char('0'))
                                .toUpper();

          if (mac_str == ACT_BROADCAST_MAC_ADDRESS) {
            continue;  // skip these entry
          }

          // // Get IPAddress
          QString ip_str;
          quint32 original_addr_number = pIpNetTable->table[i].dwAddr;
          // qToBigEndian: ip_str: 369098976(22.0.0.224) need transfeer to 3758096406(224.0.0.22)
          ActIpv4::AddressNumberToStr(qToBigEndian(original_addr_number), ip_str);

          // qDebug() << __func__ << QString("IP: %1, MAC: %2").arg(ip_str).arg(mac_str);
          // Insert Arp entry
          ip_mac_map_result.insert(ip_str, mac_str);
        }
      }
    }
  } catch (std::exception &e) {
    qCritical() << __func__ << "Get Win32 ArpTable failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }
#else
  const int size = 256;
  char line[size];
  char ip_address[size], mac_address[size], type[4];

  QString arp_file = "/proc/net/arp";
  QFile inputFile(arp_file);
  // qDebug() << __func__ << "open file.";
  if (inputFile.open(QIODevice::ReadOnly)) {
    // qDebug() << __func__ << "open success.";

    QTextStream in(&inputFile);
    QString line = in.readLine();
    while (!line.isNull()) {
      sscanf(line.toStdString().c_str(), "%s 0x%*s 0x%s %s %*s %*s\n", ip_address, type, mac_address);
      QString ip_str(ip_address), mac_str(mac_address), type_str(type);

      // Transfer type
      // 0x0 incomplete
      // 0x2 complete -> ACT_ARP_ENTRY_DYNAMIC
      // 0x6 complete and manually set -> ACT_ARP_ENTRY_STATIC
      if (type_str == "2") {
        type_str = ACT_ARP_ENTRY_DYNAMIC;
      } else if (type_str == "6") {
        type_str = ACT_ARP_ENTRY_STATIC;
      }

      if (!arp_entry_types.contains(type_str)) {
        line = in.readLine();
        continue;
      }

      // MAC format transfer
      mac_str.replace(":", "-");    // 00:15:5d:37:eb:0e -> 00-15-5d-37-eb-0e
      mac_str = mac_str.toUpper();  // 00-15-5d-37-eb-0e -> 00-15-5D-37-EB-0E
      if (mac_str == ACT_BROADCAST_MAC_ADDRESS) {
        line = in.readLine();
        continue;  // skip these entry
      }

      // qDebug() << __func__ << QString("IP:%1, MAC:%2, Type: %3").arg(ip_address).arg(mac_address).arg(type);

      // Insert Arp entry
      ip_mac_map_result.insert(ip_str, mac_str);

      line = in.readLine();
    }

    inputFile.close();
  }

#endif
  return act_status;
}
ACT_STATUS ActSouthbound::DeleteLocalhostArpEntry(const ActDevice &device) {
  ACT_STATUS_INIT();
#ifdef _WIN32
  ActNewMoxaCommandHandler new_moxa_command;
  act_status = new_moxa_command.DeleteArpEntry(device);
#else
  QMap<QString, QString> ip_dev_map_result;
  act_status = GetLocalhostAdapterIpDeviceTable(ip_dev_map_result);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostAdapterIpDeviceTable() failed.";
    return act_status;
  }
  act_status = DeleteArpEntry(device.GetIpv4().GetIpAddress());
#endif

  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << QString("DeleteArpEntry() Failed.").toStdString().c_str();
  }

  // Get Localhost ARP table
  QMap<QString, QString> arp_table;
  QSet<QString> arp_entry_types;
  arp_entry_types << ACT_ARP_ENTRY_STATIC;  // static
  act_status = GetLocalhostArpTable(arp_entry_types, arp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostArpTable() failed.";
    return act_status;
  }

  // Check arp table
  if (arp_table.contains(device.GetIpv4().GetIpAddress())) {  // delete fail
    qCritical() << __func__
                << QString("DeleteArpEntry() Failed. The arp entry(%1:%2) still exist.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .arg(arp_table[device.GetIpv4().GetIpAddress()])
                       .toStdString()
                       .c_str();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }

  return act_status;
}

ACT_STATUS ActSouthbound::SetLocalhostArpTable(const ActDevice &device, const QString &localhost_ip) {
  ACT_STATUS_INIT();

  // qDebug() << __func__ << "Localhost IP:" << localhost_ip;

  // Check localhost mac is valid
  if (!IsActStatusSuccess(CheckMacAddress(device.GetMacAddress()))) {
    qCritical() << __func__
                << QString("SetLocalhostArpTable() Failed. The arp entry(%1:%2) MAC is invalid.")
                       .arg(device.GetIpv4().GetIpAddress())
                       .arg(device.GetMacAddress())
                       .toStdString()
                       .c_str();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }

  // Get LocalAdapterTable
  QMap<QString, QString> ip_dev_map_result;
  act_status = GetLocalhostAdapterIpDeviceTable(ip_dev_map_result);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostAdapterIpDeviceTable() failed.";
    return act_status;
  }

  // [bugfix:1739] TSN-G5004、TSN-G5008 with FW2.1 execute configuration wizard unlock failed
  // FW2.1 response host value has wrong. So if the localhost_ip not in the ACT's adapter table would add the arp_entry
  // to all adapters.
  QSet<QString> target_adapter_ip_set;
  if (!ip_dev_map_result.contains(localhost_ip)) {  // localhost_ip not in the ACT's adapter table
    // Add the arp_entry to all adapters
    foreach (auto ip, ip_dev_map_result.keys()) {
      target_adapter_ip_set.insert(ip);
    }
  } else {
    target_adapter_ip_set.insert(localhost_ip);
  }

  // Set Arp Table
  for (auto target_adapter_ip : target_adapter_ip_set) {
    // qDebug() << __func__
    //          << QString("Add arp_entry(ID:%1, IP:%2, MAC:%3) to target_adapter(%4)")
    //                 .arg(device.GetId())
    //                 .arg(device.GetIpv4().GetIpAddress())
    //                 .arg(device.GetMacAddress())
    //                 .arg(target_adapter_ip)
    //                 .toStdString()
    //                 .c_str();

#ifdef _WIN32
    ActNewMoxaCommandHandler new_moxa_command;
    act_status = new_moxa_command.SetArpTable(device, target_adapter_ip);
#else
    act_status =
        AddArpEntry(ip_dev_map_result[target_adapter_ip], device.GetIpv4().GetIpAddress(), device.GetMacAddress());
#endif
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << QString("SetArpTable() Failed.(Adapter: %1)").arg(target_adapter_ip).toStdString().c_str();
    }
  }

  // Get Localhost ARP table
  QMap<QString, QString> arp_table;
  QSet<QString> arp_entry_types;
  arp_entry_types << ACT_ARP_ENTRY_STATIC;  // static
  act_status = GetLocalhostArpTable(arp_entry_types, arp_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostArpTable() failed.";
    return act_status;
  }

  // qDebug() << __func__ << "ARP_table:";
  // foreach (auto ip, arp_table.keys()) {  // mac: 38-F3-AB-E2-69-67
  //   qDebug() << __func__ << QString("ip: %1, mac:%2").arg(ip).arg(arp_table[ip]).toStdString().c_str();
  // }

  // Check arp table
  if (arp_table[device.GetIpv4().GetIpAddress()] != device.GetMacAddress()) {
    qCritical() << __func__
                << QString("SetArpTable() Failed. Check ArpTable error. %1:%2 not change to %3")
                       .arg(device.GetIpv4().GetIpAddress())
                       .arg(arp_table[device.GetIpv4().GetIpAddress()])
                       .arg(device.GetMacAddress())
                       .toStdString()
                       .c_str();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }

  // SLEEP_MS(1000);
  return act_status;
}

ACT_STATUS ActSouthbound::FindSourceDeviceByLLDPAndMacTable(const QList<ActDevice> &devices,
                                                            ActSourceDevice &result_src_device) {
  ACT_STATUS_INIT();
  result_src_device.SetDeviceId(-1);
  result_src_device.SetInterfaceId(-1);

  // Find ACT's neighbor switch(source device)
  // Get Host adapters mac
  QMap<QString, QString> adapter_table;
  act_status = GetLocalhostAdapterTable(adapter_table);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetLocalhostAdapterTable() failed.";
    return act_status;
  }

  // Find source device by LldpRemoteMac
  QList<QString> adapters_mac_list(adapter_table.values());
  QSet<QString> adapters_mac_set(adapters_mac_list.begin(), adapters_mac_list.end());

  // First find by LLDP for each device
  for (auto device : devices) {
    // Find Device's LLDP Data
    for (auto port_id : device.lldp_data_.GetRemPortChassisIdMap().keys()) {
      auto rem_port_chassis_id = device.lldp_data_.GetRemPortChassisIdMap()[port_id];
      if (rem_port_chassis_id.isEmpty()) {
        continue;
      }

      // Try to transfer to MAC, not all chassis_id is the MAC
      QString rem_mac = "";

      // [bugfix:3699] Southbound - Check the subtype of the chassid
      qint64 rem_port_chassis_id_subtype = -1;
      if (device.lldp_data_.GetRemPortChassisIdSubtypeMap().contains(port_id)) {
        rem_port_chassis_id_subtype = device.lldp_data_.GetRemPortChassisIdSubtypeMap()[port_id];
      }
      // Chassis component(1) || MAC address(4) || Locally assigned(7)
      if (rem_port_chassis_id_subtype == 1 || rem_port_chassis_id_subtype == 4 || rem_port_chassis_id_subtype == 7) {
        auto transfer_status = TransferChassisIdToMacFormat(rem_port_chassis_id, rem_mac);
        if (!IsActStatusSuccess(transfer_status)) {
          continue;
        }
      }

      // Find adapter by remote_mac
      if (adapters_mac_set.contains(rem_mac)) {  // found
        result_src_device.SetDeviceId(device.GetId());
        result_src_device.SetInterfaceId(port_id);
        return act_status;
      }
    }
  }

  // Second find by MAC table for each device
  for (auto device : devices) {
    //  Find Device's SingleEntryMacTable
    for (auto port_id : device.single_entry_mac_table_.keys()) {
      QString rem_mac = device.single_entry_mac_table_[port_id];

      if (rem_mac.isEmpty()) {
        continue;
      }

      // Find adapter by remote_mac
      if (adapters_mac_set.contains(rem_mac)) {  // found
        result_src_device.SetDeviceId(device.GetId());
        result_src_device.SetInterfaceId(port_id);
        return act_status;
      }
    }
  }

  return std::make_shared<ActStatusNotFound>("ACT location");
}

ACT_STATUS ActSouthbound::ParserMacForLinuxBase(const QString &mac, char *dst) {
  ACT_STATUS_INIT();
  // qDebug() << __func__ << QString("mac: %1, dst: %2").arg(mac).arg(dst).toStdString().c_str();
#ifdef __linux__

  // MAC format transfer
  QString mac_str(mac);
  mac_str.replace("-", ":");    // 00-15-5D-37-EB-0E -> 00:15:5D:37:EB:0E
  mac_str = mac_str.toLower();  // 00:15:5D:37:EB:0E -> 00:15:5d:37:eb:0e

  auto mac_str_list = mac_str.split(":");
  if (mac_str_list.size() != 6) {  // length invalid
    qCritical() << __func__ << QString("Invalid MAC address(%1). Length invalid.").arg(mac).toStdString().c_str();
    return std::make_shared<ActStatusInternalError>("Southbound");
  }

  int i = 0;
  unsigned byte;
  for (auto mac_s : mac_str_list) {
    if (sscanf(mac_s.toStdString().c_str(), "%x", &byte) != 1 || byte > 0xff) {  // invalid
      qCritical() << __func__ << QString("Invalid MAC address(%1).").arg(mac).toStdString().c_str();
      return std::make_shared<ActStatusInternalError>("Southbound");
    }

    *(dst + i) = byte;
    i++;
  }

  // qDebug() << __func__ << QString("dst: %1, length: %2").arg(dst).arg(i).toStdString().c_str();

#endif

  return act_status;
}
ACT_STATUS ActSouthbound::AddArpEntry(const QString &interface_name, const QString &ip, const QString &mac) {
  ACT_STATUS_INIT();
  // qDebug() << __func__
  //          << QString("interface_name: %1, ip: %2, mac:
  //          %3").arg(interface_name).arg(ip).arg(mac).toStdString().c_str();

#ifdef __linux__
  struct arpreq arp_req;
  struct sockaddr_in *sin;
  QString ip_str(ip);
  QString if_name_str(interface_name);

  sin = (struct sockaddr_in *)&(arp_req.arp_pa);
  memset(&arp_req, 0, sizeof(arp_req));
  sin->sin_family = AF_INET;

  // Set IP address
  inet_pton(AF_INET, (char *)ip_str.toStdString().c_str(), &(sin->sin_addr));

  // Set Interface name
  strncpy(arp_req.arp_dev, (char *)if_name_str.toStdString().c_str(), IFNAMSIZ - 1);

  // Set MAC
  act_status = ParserMacForLinuxBase(mac, arp_req.arp_ha.sa_data);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << QString("ParserMacForLinuxBase() Failed.").toStdString().c_str();
    return act_status;
  }

  // Set Flags
  arp_req.arp_flags = ATF_PERM | ATF_COM;

  // Config Arp table
  int sfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (ioctl(sfd, SIOCSARP, &arp_req) < 0) {  // SIOCSARP 0x8955: set ARP table entry
    qCritical() << __func__
                << QString("Add LocalHost ARP entry failed. Error: %1, Interface: %2")
                       .arg(strerror(errno))
                       .arg(interface_name)
                       .toStdString()
                       .c_str();
    return std::make_shared<ActStatusSetConfigFailed>("Add arp entry", QString("ACT's %1").arg(interface_name));
  }
#endif
  return act_status;
}

ACT_STATUS ActSouthbound::DeleteArpEntry(const QString &ip) {
  ACT_STATUS_INIT();
  // qDebug() << __func__ << QString("ip: %1").arg(ip).toStdString().c_str();

#ifdef __linux__

  struct arpreq arp_req;
  struct sockaddr_in *sin;
  QString ip_str(ip);
  // QString if_name_str(interface_name);

  sin = (struct sockaddr_in *)&(arp_req.arp_pa);
  memset(&arp_req, 0, sizeof(arp_req));
  sin->sin_family = AF_INET;

  // Set IP address
  inet_pton(AF_INET, (char *)ip_str.toStdString().c_str(), &(sin->sin_addr));

  // Config Arp table
  int sfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (ioctl(sfd, SIOCDARP, &arp_req) < 0) {  // SIOCDARP 0x8953: delete ARP table entry
    qDebug() << "Delete ARP entry failed:" << strerror(errno);
    exit(EXIT_FAILURE);
  }

#endif
  return act_status;
}

// // Old southbound currently not refactor, because current not use this function.
// ACT_STATUS ActSouthbound::GetDeviceIfOperationalStatus(const ActDevice &device,
//                                                        QMap<qint64, quint8> &result_port_speed_map) {
//   // INTEGER up(1), down(2), testing(3), unknown(4), dormant(5), notPresent(6), lowerLayerDown(7)

//   ACT_STATUS_INIT();
//   ActSnmpHandler snmp_handler;
//   result_port_speed_map.clear();

//   QSet<ActPortOidValue> port_status_entry_set;
//   act_status = snmp_handler.GetIfOperStatus(device, port_status_entry_set);
//   if (!IsActStatusSuccess(act_status)) {  // assign empty set
//     return GetDataFailProtocolErrorHandler(__func__, device, ActConnectProtocolTypeEnum::kSNMP,
//                                            "Intertface OperStatus");
//   }

//   for (auto entry : port_status_entry_set) {
//     quint16 south_status = static_cast<quint16>(entry.GetValue().toUInt());
//     result_port_speed_map.insert(entry.GetPortOid().toInt(), south_status);
//   }

//   return act_status;
// }
// // Old southbound currently not refactor, because current not use this function.
// ACT_STATUS ActSouthbound::GetDeviceIfPropagationDelays(const ActDevice &device,
//                                                        QMap<qint64, quint32> &result_port_prop_delay_map) {
//   ACT_STATUS_INIT();
//   ActSnmpHandler snmp_handler;
//   result_port_prop_delay_map.clear();

//   QSet<ActPortOidValue> port_prop_delay_entry_set;
//   act_status = snmp_handler.Get8021AsPortDSNeighborPropDelayFromMsLs(device, port_prop_delay_entry_set);
//   if (!IsActStatusSuccess(act_status)) {  // assign empty set
//     return GetDataFailProtocolErrorHandler(__func__, device, ActConnectProtocolTypeEnum::kSNMP,
//                                            "Intertface PropagationDelay");
//   }

//   for (auto entry : port_prop_delay_entry_set) {
//     quint32 south_prop_delay = entry.GetValue().toUInt();
//     result_port_prop_delay_map.insert(entry.GetPortOid().toInt(), south_prop_delay);
//   }

//   return act_status;
// }
