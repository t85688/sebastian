#include <QtEndian>

#include "act_new_moxa_command_handler.h"
#include "act_restful_client_handler.h"
#include "act_snmp_handler.h"
#include "act_southbound.hpp"
#include "act_system.hpp"
#include "act_utilities.hpp"

ACT_STATUS ActSouthbound::InsertMethodToCache(const ActFeatureMethod &feat_method) {
  ACT_STATUS_INIT();

  // SNMP
  auto snmp_str = kActConnectProtocolTypeEnumMap.key(ActConnectProtocolTypeEnum::kSNMP);
  if (feat_method.GetProtocols().contains(snmp_str)) {
    auto feat_method_protocol = feat_method.GetProtocols()[snmp_str];
    for (auto action_key : feat_method_protocol.GetActions().keys()) {
      auto action = feat_method_protocol.GetActions()[action_key];
      if (!action.GetPath().isEmpty()) {
        probe_success_oid_cache_.insert(action.GetPath());
      }
    }
  }

  return act_status;
}

bool ActSouthbound::CheckMethodHasCache(const ActFeatureMethod &feat_method) {
  // Return empty protocols
  if (feat_method.GetProtocols().isEmpty()) {
    return false;
  }

  // Check protocol is certificated (SNMP & NETCONF & RESTful & MOXAcommand)
  for (auto protocol_str : feat_method.GetProtocols().keys()) {
    if (snmp_str_ == protocol_str || netconf_str_ == protocol_str || restful_str_ == protocol_str ||
        moxa_command_str_ == protocol_str) {
      continue;
    }
    return false;
  }

  // Check SNMP
  if (feat_method.GetProtocols().contains(snmp_str_)) {
    auto feat_method_protocol = feat_method.GetProtocols()[snmp_str_];
    for (auto action_key : feat_method_protocol.GetActions().keys()) {
      auto action = feat_method_protocol.GetActions()[action_key];

      // If oid is empty would return false.
      if (action.GetPath().isEmpty()) {
        return false;
      }

      // If oid not in probe_success_oid_cache_ would return false.
      if (!probe_success_oid_cache_.contains(action.GetPath())) {
        return false;
      }
    }
  }

  // Check NETCONF
  if (feat_method.GetProtocols().contains(netconf_str_)) {
    auto feat_method_protocol = feat_method.GetProtocols()[netconf_str_];
    for (auto action_key : feat_method_protocol.GetActions().keys()) {
      auto action = feat_method_protocol.GetActions()[action_key];

      // If YANG is empty would return false.
      if (action.GetSource().isEmpty()) {
        return false;
      }

      // If YANG not in probe_yang_cache_ would return false.
      if (!probe_yang_cache_.contains(action.GetSource())) {
        return false;
      }
    }
  }

  //  Check RESTful
  if (feat_method.GetProtocols().contains(restful_str_)) {
    // RESTful not support cache
    return false;
  }

  //  Check MOXAcommand
  if (feat_method.GetProtocols().contains(moxa_command_str_)) {
    // MOXAcommand not support cache
    return false;
  }

  return true;
}

QString ActSouthbound::FormatUptime(qint64 uptime_centisecond) {  // 1/100 second
  qint64 uptime_seconds = uptime_centisecond / 100;
  qint64 days = uptime_seconds / (24 * 3600);
  qint64 hours = (uptime_seconds % (24 * 3600)) / 3600;
  qint64 minutes = (uptime_seconds % 3600) / 60;
  qint64 seconds = uptime_seconds % 60;

  return QString("%1d%2h%3m%4s").arg(days).arg(hours).arg(minutes).arg(seconds);
}

ACT_STATUS ActSouthbound::GetMethodSequence(const QString &feat_str, const ActFeatureSubItem &feat_sub_item,
                                            QList<QString> &result_method_sequence) {
  ACT_STATUS_INIT();

  result_method_sequence = feat_sub_item.GetProbeSequence();
  if (result_method_sequence.isEmpty()) {
    // Assign the Methods key list
    result_method_sequence = feat_sub_item.GetMethods().keys();
  } else {
    // Check sequence item has methods
    for (auto sequence_item : result_method_sequence) {
      if (!feat_sub_item.GetMethods().contains(sequence_item)) {
        return SequenceItemNotFoundErrorHandler(__func__, sequence_item, feat_str);
      }
    }
  }

  return act_status;
}

ACT_STATUS ActSouthbound::ActionEnableSnmpService(const bool &check_connect, const ActDevice &device,
                                                  const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("EnableSnmpService");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    if (check_connect) {
      act_status = CheckMethodProtocolsStatus(device, method);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      ActClientSnmpService snmp_service_config;

      // [bugfix:3694] After restarting the device, the SNMP service becomes abnormal
      // Short-term solution
      snmp_service_config.Setmode(2);  // disable
      act_status = restful_handler.SetSnmpService(device, "PatchSnmpService", method.GetProtocols()[restful_str_],
                                                  snmp_service_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      snmp_service_config.Setmode(1);  // enable
      act_status = restful_handler.SetSnmpService(device, "PatchSnmpService", method.GetProtocols()[restful_str_],
                                                  snmp_service_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetModelName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             QString &result_model_name) {
  ACT_STATUS_INIT();
  const QString feat_str("ModelName");
  result_model_name = "";

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpLocSysDesc", method.GetProtocols()[snmp_str_], result_model_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "SysDescr", method.GetProtocols()[snmp_str_], result_model_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_model_name.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetFirmwareVersion(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   QString &result_firmware_version) {
  ACT_STATUS_INIT();
  const QString feat_str("FirmwareVersion");
  result_firmware_version = "";

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "SiStatProductInfoFirmwareVersion",
                                            method.GetProtocols()[snmp_str_], result_firmware_version);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "IFwVersion", method.GetProtocols()[snmp_str_], result_firmware_version);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "FirmwareVersion", method.GetProtocols()[snmp_str_],
                                            result_firmware_version);

      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method4") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "FirmwareVersion", method.GetProtocols()[snmp_str_],
                                            result_firmware_version);

      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_firmware_version.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetDeviceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              QString &result_device_name) {
  ACT_STATUS_INIT();
  const QString feat_str("DeviceName");
  result_device_name = "";

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "SysName", method.GetProtocols()[snmp_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpLocSysName", method.GetProtocols()[snmp_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpV2LocSysName", method.GetProtocols()[snmp_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method4") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetDeviceName(device, "DeviceName", method.GetProtocols()[restful_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetVendorId(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            quint32 &result_vendor_id) {
  ACT_STATUS_INIT();
  const QString feat_str("VendorId");
  result_vendor_id = 0;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetSysObjectId(device, "SysObjectID", method.GetProtocols()[snmp_str_], result_vendor_id);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_vendor_id == 0) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

// ACT_STATUS ActSouthbound::ActionScanLinksByMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
//                                                     const QSet<ActDevice> &alive_devices,
//                                                     const QSet<ActLink> &exist_links,
//                                                     const QMap<QString, QString> &ip_mac_table,
//                                                     ActScanLinksResult &result) {
//   ACT_STATUS_INIT();
//   const QString feat_str("MACTable");

//   if (feat_sub_item.GetMethods().isEmpty()) {
//     return MethodsEmptyErrorHandler(__func__, device, feat_str);
//   }

//   // Get Method sequence
//   QList<QString> method_sequence;
//   act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
//   if (!IsActStatusSuccess(act_status)) {  // failed
//     return act_status;
//   }

//   QMap<qint64, QSet<QString>> port_macs_map;
//   for (auto method_key : method_sequence) {
//     auto method = feat_sub_item.GetMethods()[method_key];
//     // Check method protocols status
//     act_status = CheckMethodProtocolsStatus(device, method);
//     if (!IsActStatusSuccess(act_status)) {  // failed
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Start access
//     if (method_key == "Method1") {
//       // SNMP
//       ActSnmpHandler snmp_handler;
//       act_status =
//           snmp_handler.GetPortMacsMap(device, "Dot1qTpFdbPort", method.GetProtocols()[snmp_str_], port_macs_map);
//       if (!IsActStatusSuccess(act_status)) {  // failed
//         qDebug() << __func__ << act_status->GetErrorMessage();
//         continue;
//       }
//     } else {
//       // Not match any implemented method
//       act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Validate value
//     if (port_macs_map.isEmpty()) {
//       act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Generate result_link_set
//     QSet<ActLink> result_link_set;
//     QSet<ActDevice> result_update_device_set;
//     act_status = GenerateLinkSetByMacTable(device, alive_devices, exist_links, ip_mac_table, port_macs_map,
//                                            result_link_set, result_update_device_set);
//     if (!IsActStatusSuccess(act_status)) {
//       act_status = GenerateDataFailErrorHandler(__func__, device, "Links(MAC table)");
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Update result
//     result.SetScanLinks(result_link_set);
//     result.SetUpdateDevices(result_update_device_set);
//     // QMap<qint64, QString> result_port_mac_map;
//     // for (auto port : port_macs_map.keys()) {
//     //   if (port_macs_map[port].size() == 1) {  // only a single MAC need to assigned
//     //     result_port_mac_map[port] = *port_macs_map[port].begin();
//     //   }
//     // }
//     // result.SetPortMacMap(result_port_mac_map);
//     return ACT_STATUS_SUCCESS;
//   }

//   return AccessFailErrorHandler(__func__, device, feat_str, act_status);
// }

ACT_STATUS ActSouthbound::ActionFindSourceByMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     const QSet<QString> &adapters_mac_set, qint64 &result_port_id) {
  ACT_STATUS_INIT();
  const QString feat_str("MACTable");
  result_port_id = -1;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, QSet<QString>> port_macs_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortMacsMap(device, "Dot1qTpFdbPort", method.GetProtocols()[snmp_str_], port_macs_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (port_macs_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Find source
    // Find only one entry port
    foreach (auto port, port_macs_map.keys()) {
      // for (auto mac : port_macs_map[port]) {
      //   qDebug() << __func__
      //            << QString("dev: %1, port:%2, mac:
      //            %3").arg(device.GetId()).arg(port).arg(mac).toStdString().c_str();
      // }

      if (port_macs_map[port].size() == 1) {
        // Find adapter by remote_mac
        QString rem_mac = *port_macs_map[port].begin();
        if (adapters_mac_set.find(rem_mac) != adapters_mac_set.end()) {  // found
          result_port_id = port;
          return ACT_STATUS_SUCCESS;
        }
      }
    }
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetLldpLocChassisID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    QString &result_chassis_id) {
  ACT_STATUS_INIT();
  const QString feat_str("LLDPLocaChassisID");
  result_chassis_id = "";

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpLocChassisId", method.GetProtocols()[snmp_str_], result_chassis_id);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpV2LocChassisId", method.GetProtocols()[snmp_str_], result_chassis_id);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_chassis_id.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetInterfaceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 QMap<qint64, QString> &result_if_name_map) {
  ACT_STATUS_INIT();
  const QString feat_str("InterfaceName");
  result_if_name_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortStrMap(device, "LldpLocPortId", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortStrMap(device, "LldpV2LocPortId", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortStrMap(device, "IfName", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method4") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortStrMap(device, "IfDescr", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_if_name_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetLldpLocPortID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 QMap<qint64, QString> &result_if_port_id_map) {
  ACT_STATUS_INIT();
  const QString feat_str("LldpLocPortID");
  result_if_port_id_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortStrMap(device, "LldpLocPortId", method.GetProtocols()[snmp_str_], result_if_port_id_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortStrMap(device, "LldpV2LocPortId", method.GetProtocols()[snmp_str_],
                                              result_if_port_id_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_if_port_id_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetInterfaceMac(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                QMap<qint64, QString> &result_if_mac_map) {
  ACT_STATUS_INIT();
  const QString feat_str("InterfaceMAC");
  result_if_mac_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortMacMap(device, "IfPhysAddress", method.GetProtocols()[snmp_str_], result_if_mac_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_if_mac_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetPortSpeed(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             QMap<qint64, qint64> &result_port_speed_map) {
  ACT_STATUS_INIT();
  const QString feat_str("PortSpeed");
  result_port_speed_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      QMap<qint64, qint64> south_port_int_map;
      act_status =
          snmp_handler.GetPortIntMap(device, "IfHighSpeed", method.GetProtocols()[snmp_str_], south_port_int_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_int_map.keys()) {
        result_port_speed_map[port] = static_cast<quint32>(south_port_int_map[port]);
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      QMap<qint64, qint64> south_port_int_map;
      act_status = snmp_handler.GetPortIntMap(device, "IfSpeed", method.GetProtocols()[snmp_str_], south_port_int_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_int_map.keys()) {
        result_port_speed_map[port] = static_cast<quint32>(south_port_int_map[port]);
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_speed_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}
ACT_STATUS ActSouthbound::ActionGetLldpData(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            ActLLDPData &result_lldp_data) {
  ACT_STATUS_INIT();
  const QString feat_str("LldpData");
  ActLLDPData lldp_data;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;

      act_status = snmp_handler.GetStrValue(device, "LldpLocChassisId", method.GetProtocols()[snmp_str_],
                                            lldp_data.GetLocChassisId());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetPortStrMap(device, "LldpLocPortId", method.GetProtocols()[snmp_str_],
                                              lldp_data.GetLocPortIdMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status =
          snmp_handler.GetLldpRemPortIntMap(device, "LldpRemChassisIdSubtype", method.GetProtocols()[snmp_str_],
                                            lldp_data.GetRemPortChassisIdSubtypeMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortStrMap(device, "LldpRemChassisId", method.GetProtocols()[snmp_str_],
                                                     lldp_data.GetRemPortChassisIdMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortIntMap(device, "LldpRemPortIdSubtype", method.GetProtocols()[snmp_str_],
                                                     lldp_data.GetRemPortIdSubtypeMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortStrMap(device, "LldpRemPortId", method.GetProtocols()[snmp_str_],
                                                     lldp_data.GetRemPortIdMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "LldpV2LocChassisId", method.GetProtocols()[snmp_str_],
                                            lldp_data.GetLocChassisId());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetPortStrMap(device, "LldpV2LocPortId", method.GetProtocols()[snmp_str_],
                                              lldp_data.GetLocPortIdMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status =
          snmp_handler.GetLldpRemPortIntMap(device, "LldpV2RemChassisIdSubtype", method.GetProtocols()[snmp_str_],
                                            lldp_data.GetRemPortChassisIdSubtypeMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortStrMap(device, "LldpV2RemChassisId", method.GetProtocols()[snmp_str_],
                                                     lldp_data.GetRemPortChassisIdMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortIntMap(device, "LldpV2RemPortIdSubtype", method.GetProtocols()[snmp_str_],
                                                     lldp_data.GetRemPortIdSubtypeMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortStrMap(device, "LldpV2RemPortId", method.GetProtocols()[snmp_str_],
                                                     lldp_data.GetRemPortIdMap());
      if (stop_flag_) {
        return ACT_STATUS_STOP;
      }
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (lldp_data.GetLocChassisId().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "LocChassisId");
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (lldp_data.GetLocPortIdMap().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "LocPortId");
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (lldp_data.GetRemPortChassisIdSubtypeMap().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemChassisIDSubtype");
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    if (lldp_data.GetRemPortChassisIdMap().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemChassisID");
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    if (lldp_data.GetRemPortIdSubtypeMap().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemPortIdSubtype");
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    if (lldp_data.GetRemPortIdMap().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemPortId");
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Update result
    result_lldp_data = lldp_data;
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetSingleEntryMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                       QMap<qint64, QString> &result_port_mac_map) {
  ACT_STATUS_INIT();
  const QString feat_str("MACTable");
  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  result_port_mac_map.clear();

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, QSet<QString>> port_macs_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortMacsMap(device, "Dot1qTpFdbPort", method.GetProtocols()[snmp_str_], port_macs_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (port_macs_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    for (auto port : port_macs_map.keys()) {
      if (port_macs_map[port].size() == 1) {  // only a single MAC need to assigned
        result_port_mac_map[port] = *port_macs_map[port].begin();
      }
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionCheckSnmpConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("SNMP connect");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      quint32 vendor_id = 0;
      act_status = snmp_handler.GetSysObjectId(device, "SysObjectID", method.GetProtocols()[snmp_str_], vendor_id);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      // [bugfix:3694] After restarting the device, the SNMP service becomes abnormal
      // Short-term solution
      QString sys_descr;
      act_status = snmp_handler.GetStrValue(device, "SysDescr", method.GetProtocols()[snmp_str_], sys_descr);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      if (sys_descr.contains("aarch64")) {  // failed
        QString error_msg = QString("Device SNMP service abnormal");
        act_status = std::make_shared<ActBadRequest>(error_msg);
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionCheckRestfulConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("RESTful connect");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];

    // Start access
    if (method_key == "Method1") {
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.CheckConnect(device, "Login", method.GetProtocols()[restful_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionCheckNewMOXACommandConnect(const ActDevice &device,
                                                           const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("NewMOXACommand connect");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];

    // Start access
    if (method_key == "Method1") {
      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status =
          new_moxa_command_handler.VerifyAccount(device, "VerifyAccount", method.GetProtocols()[moxa_command_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::GetTSNSwitchConfigurationSyncStatus(const ActDevice &device, bool &check_result) {
  ACT_STATUS_INIT();

  ActRestfulClientHandler restful_handler;
  act_status = restful_handler.GetDeviceConfigurationSyncStatus(device, check_result);
  if (!IsActStatusSuccess(act_status)) {  // failed
    qDebug() << __func__ << act_status->GetErrorMessage();
    return act_status;
  }
  return act_status;
}

ACT_STATUS ActSouthbound::ActionReboot(const ActDevice &device, const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("Reboot");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.Reboot(device, "PostReboot", method.GetProtocols()[restful_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP MOXA-TWS3000-APL-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetEnableWarmStart(device, "EnableWarmStart", method.GetProtocols()[snmp_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // SNMP MOXA-TWS3000-SPE-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetEnableWarmStart(device, "EnableWarmStart", method.GetProtocols()[snmp_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionFactoryDefault(const ActDevice &device, const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("FactoryDefault");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful

      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.FactoryDefault(device, "PostFactoryDefault", method.GetProtocols()[restful_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP MOXA-TWS3000-APL-MIB
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.SetEnableFactoryDefault(device, "EnableFactoryDefault", method.GetProtocols()[snmp_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // SNMP MOXA-TWS3000-SPE-MIB
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.SetEnableFactoryDefault(device, "EnableFactoryDefault", method.GetProtocols()[snmp_str_]);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionFirmwareUpgrade(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                const QString &file_path) {
  ACT_STATUS_INIT();
  const QString feat_str("FirmwareUpgrade");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // MoxaIEI-MoxaCommand
      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status = new_moxa_command_handler.UpgradeFirmware(device, "UpgradeFirmware",
                                                            method.GetProtocols()[moxa_command_str_], file_path);

      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // If the model name is EDS series, use RESTful to improve upgrade firmware speed
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.PreStartImport(device, "PostPreStartImport", method.GetProtocols()[restful_str_]);
      // Not all series support this API, so if failed, just use MoxaCommand to upgrade firmware directly
      if (!IsActStatusSuccess(act_status)) {  // failed
        qWarning() << __func__ << act_status->GetErrorMessage();
      }

      // MoxaIEI-MoxaCommand
      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status = new_moxa_command_handler.UpgradeFirmware(device, "UpgradeFirmware",
                                                            method.GetProtocols()[moxa_command_str_], file_path);

      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionExportConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             const QString &file_path) {
  ACT_STATUS_INIT();
  const QString feat_str("ImportExport");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // New MOXA command
      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status = new_moxa_command_handler.ExportConfig(device, "ImportExport",
                                                         method.GetProtocols()[moxa_command_str_], file_path);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.ExportConfig(device, "Export", method.GetProtocols()[restful_str_], file_path);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // [bugfix:3993] Operation > Export/Import Device Config failed
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.ExportConfig(device, "Export", method.GetProtocols()[restful_str_], file_path);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionImportConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             const QString &file_path) {
  ACT_STATUS_INIT();
  const QString feat_str("ImportExport");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // New MOXA command
      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status = new_moxa_command_handler.ImportConfig(device, "ImportExport",
                                                         method.GetProtocols()[moxa_command_str_], file_path);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.ImportConfig(device, "Import", method.GetProtocols()[restful_str_], file_path);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // [bugfix:3993] Operation > Export/Import Device Config failed
      // New MOXA command
      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status =
          new_moxa_command_handler.ImportConfig(device, "Import", method.GetProtocols()[moxa_command_str_], file_path);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionLocator(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        const quint16 &duration) {
  ACT_STATUS_INIT();
  const QString feat_str("Locator");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.Locator(device, "PostLocator", method.GetProtocols()[restful_str_], duration);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // } else if (method_key == "Method2") {
      // // SNMP MOXA-LOCATOR-MIB
      // ActSnmpHandler snmp_handler;
      // act_status = snmp_handler.SetLocatorConfigActivate(device, "LocatorConfigActivate",
      // method.GetProtocols()[snmp_str_]); if (!IsActStatusSuccess(act_status)) {  // failed
      //   qDebug() << __func__ << act_status->GetErrorMessage();
      //   continue;
      // }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetEventLog(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            ActDeviceEventLog &result_event_log) {
  ACT_STATUS_INIT();
  const QString feat_str("EventLog");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetEventLog(device, "LogEntry", method.GetProtocols()[restful_str_], result_event_log);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionBroadcastSearchDevices(const ActFeatureSubItem &feat_sub_item,
                                                       QList<ActDevice> &result_device_list,
                                                       QMap<QString, QString> &result_mac_host_map) {
  ACT_STATUS_INIT();
  const QString feat_str("BroadcastSearch");
  result_device_list.clear();
  result_mac_host_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    QString error_msg = QString("The %1 methods are Empty.").arg(feat_str);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusSouthboundFailed>(error_msg);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Start access
    if (method_key == "Method1") {
      // MoxaIEI-MoxaCommand

      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status = new_moxa_command_handler.SearchDevices("SearchDevices", method.GetProtocols()[moxa_command_str_],
                                                          result_device_list, result_mac_host_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      qDebug() << __func__ << QString("The method(%1) not implemented at %2 feature.\n ").arg(method_key).arg(feat_str);
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  // return  AccessFailError
  QString error_msg = QString("Access the %1 feature(sub-item) failed").arg(feat_str);
  qCritical() << __func__ << error_msg.toStdString().c_str();
  return std::make_shared<ActStatusSouthboundFailed>(error_msg);
}

ACT_STATUS ActSouthbound::ActionSetNetworkSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  const ActNetworkSettingTable &network_setting_table) {
  ACT_STATUS_INIT();
  const QString feat_str("NetworkSetting");

  // Check SubnetMask
  if (network_setting_table.GetSubnetMask().isEmpty()) {
    return ConfigurationCheckErrorHandler(__func__, device, QString("SubnetMask is empty"));
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // MoxaIEI-MoxaCommand

      ActNewMoxaCommandHandler new_moxa_command_handler;
      act_status = new_moxa_command_handler.ChangeNetworkSettings(
          device, "ChangeNetworkSettings", method.GetProtocols()[moxa_command_str_], network_setting_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetVLAN(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        const ActVlanStaticTable &vlan_static_table) {
  ACT_STATUS_INIT();
  const QString feat_str("VLAN Method(VLAN static)");

  // If table is empty
  if (vlan_static_table.GetVlanStaticEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method2") {
      // // SNMP Q-BRIDGE-MIB
      // ActSnmpHandler snmp_handler;
      // act_status = snmp_handler.SetDot1qVlanStatic(device, method.GetProtocols()[snmp_str_],
      //                                              vlan_static_table.GetVlanStaticEntries());
      // if (!IsActStatusSuccess(act_status)) {  // failed
      //   qDebug() << __func__ << act_status->GetErrorMessage();
      //   continue;
      // }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      ActVlanStaticTable new_vlan_static_table = vlan_static_table;
      act_status = restful_handler.SetStdVlanTable(device, "StdVlanTable", method.GetProtocols()[restful_str_],
                                                   new_vlan_static_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionDeleteVLANMember(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 const QList<qint32> &delete_vlan_list) {
  ACT_STATUS_INIT();
  const QString feat_str("VLAN Method(Delete VLAN member)");

  // If table is empty
  if (delete_vlan_list.isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method2") {
      // // SNMP Q-BRIDGE-MIB
      // ActSnmpHandler snmp_handler;
      // act_status = snmp_handler.DeleteDot1qVlanStaticMember(device, method.GetProtocols()[snmp_str_],
      //                                                    delete_vlan_list);
      // if (!IsActStatusSuccess(act_status)) {  // failed
      //   qDebug() << __func__ << act_status->GetErrorMessage();
      //   continue;
      // }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.DeleteStdVlanMember(device, "StdVlanTable", method.GetProtocols()[restful_str_],
                                                       delete_vlan_list);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionAddVLANMember(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              const QList<qint32> &add_vlan_list) {
  ACT_STATUS_INIT();
  const QString feat_str("VLAN Method(Add VLAN member)");

  // If table is empty
  if (add_vlan_list.isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method2") {
      // // SNMP Q-BRIDGE-MIB
      // ActSnmpHandler snmp_handler;
      // act_status = snmp_handler.AddDot1qVlanStaticMember(device, method.GetProtocols()[snmp_str_],
      //                                                    add_vlan_list);
      // if (!IsActStatusSuccess(act_status)) {  // failed
      //   qDebug() << __func__ << act_status->GetErrorMessage();
      //   continue;
      // }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.AddStdVlanMember(device, "StdVlanTable", method.GetProtocols()[restful_str_], add_vlan_list);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetVLAN(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                        ActVlanStaticTable &result_vlan_static_table) {
  ACT_STATUS_INIT();
  const QString feat_str("VLAN Method(VLAN static)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method2") {
      // SNMP Q-BRIDGE-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetDot1qVlanStatic(device, method.GetProtocols()[snmp_str_], vlan_static_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      ActVlanStaticTable vlan_static_table;
      act_status = restful_handler.GetStdVlanTable(device, "StdVlanTable", method.GetProtocols()[restful_str_],
                                                   vlan_static_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      vlan_static_entries = vlan_static_table.GetVlanStaticEntries();

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Set result table
    result_vlan_static_table.SetVlanStaticEntries(vlan_static_entries);
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetVLANPortType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                const ActVlanPortTypeTable &vlan_port_type_table) {
  ACT_STATUS_INIT();
  const QString feat_str("VLANPortType(Access/Trunk/Hybrid)");

  // If table is empty
  if (vlan_port_type_table.GetVlanPortTypeEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetVlanPortType(device, "VlanConfigVlanPortType", method.GetProtocols()[snmp_str_],
                                                vlan_port_type_table.GetVlanPortTypeEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      // MOXA-TWS3000-APL-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetPortVlanType(device, "PortVlanType", method.GetProtocols()[snmp_str_],
                                                vlan_port_type_table.GetVlanPortTypeEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetVlanPortType(device, "VlanPortType", method.GetProtocols()[restful_str_],
                                                   vlan_port_type_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method4") {
      // SNMP
      // MOXA-TWS3000-APL-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetPortVlanType(device, "PortVlanType", method.GetProtocols()[snmp_str_],
                                                vlan_port_type_table.GetVlanPortTypeEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetVLANPortType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                ActVlanPortTypeTable &result_vlan_port_type_table) {
  ACT_STATUS_INIT();
  const QString feat_str("VLANPortType(Access/Trunk/Hybrid)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanPortTypeEntry> vlan_port_type_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetVlanPortType(device, "VlanConfigVlanPortType", method.GetProtocols()[snmp_str_],
                                                vlan_port_type_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortVlanType(device, "PortVlanType", method.GetProtocols()[snmp_str_],
                                                vlan_port_type_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      ActVlanPortTypeTable vlan_port_type_table;
      act_status = restful_handler.GetVlanPortType(device, "VlanPortType", method.GetProtocols()[restful_str_],
                                                   vlan_port_type_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      vlan_port_type_entries = vlan_port_type_table.GetVlanPortTypeEntries();
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (vlan_port_type_entries.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Set result table
    result_vlan_port_type_table.SetVlanPortTypeEntries(vlan_port_type_entries);
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetPortPVID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            const ActPortVlanTable &port_vlan_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PortPVID");

  // If table is empty
  if (port_vlan_table.GetPortVlanEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetPortPVID(device, "Dot1qPvid", method.GetProtocols()[snmp_str_],
                                            port_vlan_table.GetPortVlanEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      // MOXA-TWS3000-APL-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetPortPVID(device, "PortDefaultVid", method.GetProtocols()[snmp_str_],
                                            port_vlan_table.GetPortVlanEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetStdVlanPVID(device, "StdVlanPVID", method.GetProtocols()[restful_str_], port_vlan_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method4") {
      // SNMP
      // MOXA-TWS3000-SPE-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetPortPVID(device, "PortDefaultVid", method.GetProtocols()[snmp_str_],
                                            port_vlan_table.GetPortVlanEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetPortPVID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            ActPortVlanTable &result_port_vlan_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PortPVID");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActPortVlanEntry> port_vlan_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortPVID(device, "Dot1qPvid", method.GetProtocols()[snmp_str_], port_vlan_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortPVID(device, "PortDefaultVid", method.GetProtocols()[snmp_str_], port_vlan_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      ActPortVlanTable port_vlan_table;
      act_status =
          restful_handler.GetStdVlanPVID(device, "StdVlanPVID", method.GetProtocols()[restful_str_], port_vlan_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      port_vlan_entries = port_vlan_table.GetPortVlanEntries();
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (port_vlan_entries.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Set result table
    result_port_vlan_table.SetPortVlanEntries(port_vlan_entries);
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetPortDefaultPCP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  const ActDefaultPriorityTable &default_priority_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PortDefaultPCP");

  // If table is empty
  if (default_priority_table.GetDefaultPriorityEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetPortPCP(device, "QosConfigDefaultPriorityValue", method.GetProtocols()[snmp_str_],
                                           default_priority_table.GetDefaultPriorityEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetQosDefaultPriority(device, "DefaultPriority", method.GetProtocols()[restful_str_],
                                                         default_priority_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetPortDefaultPCP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  ActDefaultPriorityTable &result_default_priority_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PortDefaultPCP");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActDefaultPriorityEntry> default_priority_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortPCP(device, "QosConfigDefaultPriorityValue", method.GetProtocols()[snmp_str_],
                                           default_priority_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      result_default_priority_table.SetDefaultPriorityEntries(default_priority_entries);

    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetQosDefaultPriority(device, "DefaultPriority", method.GetProtocols()[restful_str_],
                                                         result_default_priority_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_default_priority_table.GetDefaultPriorityEntries().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetManagementVlan(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  const qint32 &mgmt_vlan) {
  ACT_STATUS_INIT();
  const QString feat_str("ManagementVLAN");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetManagementVlan(device, "MgmtVlan", method.GetProtocols()[restful_str_], mgmt_vlan);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetManagementVlan(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  qint32 &result_mgmt_vlan) {
  ACT_STATUS_INIT();
  const QString feat_str("ManagementVLAN");
  result_mgmt_vlan = 0;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetManagementVlan(device, "MgmtVlan", method.GetProtocols()[restful_str_], result_mgmt_vlan);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_mgmt_vlan == 0) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetStreamPriorityIngress(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         ActStadPortTable &result_stad_port_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PerStreamPriority(Ingress)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetStreamAdapterIngress(device, "StreamAdapter", method.GetProtocols()[restful_str_],
                                                           result_stad_port_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetStreamPriorityIngress(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         const ActStadPortTable &stad_port_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PerStreamPriority(Ingress)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetStreamAdapterIngress(device, "StreamAdapter", method.GetProtocols()[restful_str_],
                                                           stad_port_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetStreamPriorityEgress(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        ActStadConfigTable &result_stad_config_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PerStreamPriority(Egress)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetStreamAdapterEgress(device, "StreamAdapter", method.GetProtocols()[restful_str_],
                                                          result_stad_config_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetStreamPriorityEgress(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        const ActStadConfigTable &stad_config_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PerStreamPriority(Egress)");

  // If table is empty
  if (stad_config_table.GetStadConfigEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetStreamAdapterEgress(device, "StreamAdapter", method.GetProtocols()[restful_str_],
                                                          stad_config_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetStaticUnicast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 const ActStaticForwardTable &static_forward_table) {
  ACT_STATUS_INIT();
  const QString feat_str("StaticForward(Unicast)");

  // If table is empty
  if (static_forward_table.GetStaticForwardEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetDot1qStaticUnicast(device, method.GetProtocols()[snmp_str_],
                                                      static_forward_table.GetStaticForwardEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetStaticUnicast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 ActStaticForwardTable &result_static_forward_table) {
  ACT_STATUS_INIT();
  const QString feat_str("StaticForward(Unicast)");
  QSet<ActStaticForwardEntry> static_forward_entries;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetDot1qStaticUnicast(device, method.GetProtocols()[snmp_str_], static_forward_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Set result table
    result_static_forward_table.SetStaticForwardEntries(static_forward_entries);
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetStaticMulticast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   const ActStaticForwardTable &static_forward_table) {
  ACT_STATUS_INIT();
  const QString feat_str("StaticForward(Multicast)");

  // If table is empty
  if (static_forward_table.GetStaticForwardEntries().isEmpty()) {
    return act_status;
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetDot1qStaticMulticast(device, method.GetProtocols()[snmp_str_],
                                                        static_forward_table.GetStaticForwardEntries());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetStaticMulticast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   ActStaticForwardTable &result_static_forward_table) {
  ACT_STATUS_INIT();
  const QString feat_str("StaticForward(Multicast)");
  QSet<ActStaticForwardEntry> static_forward_entries;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetDot1qStaticMulticast(device, method.GetProtocols()[snmp_str_], static_forward_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Set result table
    result_static_forward_table.SetStaticForwardEntries(static_forward_entries);
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeMethod(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree Method(RSTP Method)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetSpanningTreeMethod(device, method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      // MOXA-TWS3000-APL-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetTWSSpanningTreeMethod(device, method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;

      act_status = restful_handler.SetLayer2Redundancy(device, "Stprstp", method.GetProtocols()[restful_str_],
                                                       rstp_table.GetActive());
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = restful_handler.SetSpanningTree(device, "SpanningTreeVersion", method.GetProtocols()[restful_str_],
                                                   rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method4") {
      // SNMP
      // MOXA-TWS3000-SPE-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetTWSSpanningTreeMethod(device, method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeBasicRstp(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(Basic RSTP)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "BasicRstp", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = restful_handler.SetSpanningTree(device, "AutoEdge", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeHelloTime(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(HelloTime)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetIeee8021SpanningTreeBridgeHelloTime(device, "Ieee8021SpanningTreeBridgeHelloTime",
                                                                       method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetSpanningTreeHelloTime(device, "SpanningTreeHelloTime",
                                                         method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "HelloTime", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreePriority(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(Priority)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetIeee8021SpanningTreePriority(device, "Ieee8021SpanningTreePriority",
                                                                method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetSpanningTreePriority(device, "SpanningTreeBridgePriority",
                                                        method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetSpanningTree(device, "Priority", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeMaxAge(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(MaxAge)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetSpanningTree(device, "MaxAge", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeForwardDelay(const ActDevice &device,
                                                            const ActFeatureSubItem &feat_sub_item,
                                                            const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(ForwardDelay)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "ForwardDelay", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeErrorRecoveryTime(const ActDevice &device,
                                                                 const ActFeatureSubItem &feat_sub_item,
                                                                 const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(ErrorRecoveryTime)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetSpanningTree(device, "RstpErrorRecoveryTime", method.GetProtocols()[restful_str_],
                                                   rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeSwift(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(Swift)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "RstpConfigSwift", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status =
          restful_handler.SetSpanningTree(device, "RstpConfigRevert", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreePortRSTPEnable(const ActDevice &device,
                                                              const ActFeatureSubItem &feat_sub_item,
                                                              const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(PortRSTPEnable)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "RstpEnable", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreePortPriority(const ActDevice &device,
                                                            const ActFeatureSubItem &feat_sub_item,
                                                            const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(PortPriority)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "PortPriority", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeEdge(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(Edge)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "ForceEdge", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      act_status = restful_handler.SetSpanningTree(device, "AutoEdge", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreePathCost(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(PathCost)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetSpanningTree(device, "PathCost", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeLinkType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(LinkType)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "BridgeLinkType", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeBPDUGuard(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(BPDUGuard)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "BpduGuard", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeRootGuard(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(RootGuard)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.SetSpanningTreeRootGuard(device, "RstpConfigPortRootGuard",
                                                         method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "RootGuard", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeLoopGuard(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(LoopGuard)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "LoopGuard", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSpanningTreeBPDUFilter(const ActDevice &device,
                                                          const ActFeatureSubItem &feat_sub_item,
                                                          const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(BPDUFilter)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSpanningTree(device, "BpduFilter", method.GetProtocols()[restful_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetSpanningTree(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                ActRstpTable &result_rstp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method3") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetSpanningTree(device, "Stprstp", method.GetProtocols()[restful_str_], result_rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetUserAccount(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               const ActUserAccountTable &user_account_table) {
  ACT_STATUS_INIT();
  const QString feat_str("UserAccount");

  // Check Accounts not empty
  if (user_account_table.GetAccounts().isEmpty()) {
    return ConfigurationCheckErrorHandler(__func__, device, QString("The UserAccount is empty"));
  }

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetUserAccount(device, "UserAccount", method.GetProtocols()[restful_str_],
                                                  user_account_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetUserAccount(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActUserAccountTable &result_user_account_table) {
  ACT_STATUS_INIT();
  const QString feat_str("UserAccount");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetUserAccount(device, "UserAccount", method.GetProtocols()[restful_str_],
                                                  result_user_account_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetLoginPolicy(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               const ActLoginPolicyTable &login_policy_table) {
  ACT_STATUS_INIT();
  const QString feat_str("LoginPolicy");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetLoginPolicy(device, "LoginPolicy", method.GetProtocols()[restful_str_],
                                                  login_policy_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetLoginPolicy(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActLoginPolicyTable &result_login_policy_table) {
  ACT_STATUS_INIT();
  const QString feat_str("LoginPolicy");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetLoginPolicy(device, "LoginPolicy", method.GetProtocols()[restful_str_],
                                                  result_login_policy_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSnmpTrapSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   const ActSnmpTrapSettingTable &snmp_trap_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SnmpTrapSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSnmpTrapSetting(device, "SnmpTrap", method.GetProtocols()[restful_str_], snmp_trap_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetSnmpTrapSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   ActSnmpTrapSettingTable &result_snmp_trap_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SnmpTrapSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetSnmpTrapSetting(device, "SnmpTrap", method.GetProtocols()[restful_str_],
                                                      result_snmp_trap_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetSyslogSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 const ActSyslogSettingTable &syslog_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SyslogSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.SetSyslogSetting(device, "SyslogServer", method.GetProtocols()[restful_str_], syslog_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetSyslogSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 ActSyslogSettingTable &result_syslog_table) {
  ACT_STATUS_INIT();
  const QString feat_str("SyslogSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetSyslogSetting(device, "SyslogServer", method.GetProtocols()[restful_str_],
                                                    result_syslog_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetTimeSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               const ActTimeSettingTable &time_table) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetTsnDeviceTimeSetting(device, "TSNDeviceTimeSetting",
                                                           method.GetProtocols()[restful_str_], time_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetNosDeviceTimeSetting(device, "NOSDeviceTimeSetting",
                                                           method.GetProtocols()[restful_str_], time_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTimeSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActTimeSettingTable &result_time_table) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetTsnDeviceTimeSetting(device, "TSNDeviceTimeSetting",
                                                           method.GetProtocols()[restful_str_], result_time_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetNosDeviceTimeSetting(device, "NOSDeviceTimeSetting",
                                                           method.GetProtocols()[restful_str_], result_time_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetLoopProtection(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  const ActLoopProtectionTable &lp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("LoopProtection");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetLoopProtection(device, "MxLp", method.GetProtocols()[restful_str_], lp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetLoopProtection(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  ActLoopProtectionTable &result_lp_table) {
  ACT_STATUS_INIT();
  const QString feat_str("LoopProtection");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetLoopProtection(device, "MxLp", method.GetProtocols()[restful_str_], result_lp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetInformationSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      const ActInformationSettingTable &info_setting_table) {
  ACT_STATUS_INIT();
  const QString feat_str("InformationSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetInformationSetting(device, "SystemInformationSetting",
                                                         method.GetProtocols()[restful_str_], info_setting_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetInformationSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      ActInformationSettingTable &result_info_setting_table) {
  ACT_STATUS_INIT();
  const QString feat_str("InformationSetting");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetInformationSetting(
          device, "SystemInformationSetting", method.GetProtocols()[restful_str_], result_info_setting_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetManagementInterface(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                       const ActManagementInterfaceTable &mgmt_interface_table) {
  ACT_STATUS_INIT();
  const QString feat_str("ManagementInterface");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetManagementInterface(device, "ServiceManagement",
                                                          method.GetProtocols()[restful_str_], mgmt_interface_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetManagementInterface(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                       ActManagementInterfaceTable &result_mgmt_interface_table) {
  ACT_STATUS_INIT();
  const QString feat_str("ManagementInterface");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetManagementInterface(
          device, "ServiceManagement", method.GetProtocols()[restful_str_], result_mgmt_interface_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSetPortSettingAdminStatus(const ActDevice &device,
                                                          const ActFeatureSubItem &feat_sub_item,
                                                          const ActPortSettingTable &port_setting_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PortSetting(AdminStatus)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.SetPortSetting(device, "AdminStatus", method.GetProtocols()[restful_str_],
                                                  port_setting_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetPortSettingAdminStatus(const ActDevice &device,
                                                          const ActFeatureSubItem &feat_sub_item,
                                                          ActPortSettingTable &result_port_setting_table) {
  ACT_STATUS_INIT();
  const QString feat_str("PortSetting(AdminStatus)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetPortSettingAdminStatus(device, "AdminStatus", method.GetProtocols()[restful_str_],
                                                             south_port_admin_status_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (south_port_admin_status_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    result_port_setting_table.GetPortSettingEntries().clear();
    for (auto port : south_port_admin_status_map.keys()) {
      ActPortSettingEntry entry(port, south_port_admin_status_map[port]);
      result_port_setting_table.GetPortSettingEntries().insert(entry);
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSet802Dot1Qbv(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              const ActGclTable &gcl_table) {
  ACT_STATUS_INIT();
  const QString feat_str("802.1Qbv(GCL)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGet802Dot1Qbv(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActGclTable &result_gcl_table) {
  ACT_STATUS_INIT();
  const QString feat_str("802.1Qbv(GCL)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QMap<qint64, bool> south_port_admin_status_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionSet802Dot1CB(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             const ActCbTable &ieee_802_1cb_table) {
  ACT_STATUS_INIT();
  const QString feat_str("802.1CB(FRER & StreamIdentification)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionAssignDevicePartialInfos(ActDevice &device, const ActFeatureSubItem &feat_sub_item) {
  ACT_STATUS_INIT();
  const QString feat_str("Assign MOXADeviceInfos");

  ActDeviceInfo new_dev_info = device.GetDeviceInfo();
  ActIpv4 new_ipv4 = device.GetIpv4();
  ActDeviceProperty new_dev_prop = device.GetDeviceProperty();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      QMap<QString, QString> result_action_map;

      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetStringRequest(device, method.GetProtocols()[restful_str_], result_action_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

      // Assign result to device
      QMapIterator<QString, QString> map_it(result_action_map);
      while (map_it.hasNext()) {
        map_it.next();
        if (map_it.key() == "GetSerialNumber") {
          new_dev_info.SetSerialNumber(map_it.value());
        } else if (map_it.key() == "GetNetmask") {
          new_ipv4.SetSubnetMask(map_it.value());
        } else if (map_it.key() == "GetGateway") {
          new_ipv4.SetGateway(map_it.value());
        }
      }
      device.SetDeviceProperty(new_dev_prop);
      device.SetDeviceInfo(new_dev_info);
      device.SetIpv4(new_ipv4);

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetIPConfiguration(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   ActIpv4 &result_ipv4) {
  ACT_STATUS_INIT();
  const QString feat_str("IPConfiguration");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetIPConfiguration(device, "IPv4", method.GetProtocols()[restful_str_], result_ipv4);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetL3IPConfiguration(device, {"L3RouterId", "NetworkDns"},
                                                        method.GetProtocols()[restful_str_], result_ipv4);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetSerialNumber(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                QString &result_serial_number) {
  ACT_STATUS_INIT();
  const QString feat_str("SerialNumber");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetSerialNumber(device, "SerialNumber", method.GetProtocols()[restful_str_],
                                                   result_serial_number);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_serial_number.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetSystemUptime(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                QString &result_uptime) {
  ACT_STATUS_INIT();
  const QString feat_str("SystemUptime");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetSystemUptime(device, "Uptime", method.GetProtocols()[restful_str_], result_uptime);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      qint64 int_value = 0;
      act_status = snmp_handler.GetIntValue(device, "SysUpTime", method.GetProtocols()[snmp_str_], int_value);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      } else {
        result_uptime = FormatUptime(int_value);
      }
      // Handle Data
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_uptime.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetProductRevision(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   QString &result_product_revision) {
  ACT_STATUS_INIT();
  const QString feat_str("ProductRevision");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetProductRevision(device, "ProductRevision", method.GetProtocols()[restful_str_],
                                                      result_product_revision);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_product_revision.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetRedundantProtocol(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     QString &result_redundant_protocol) {
  ACT_STATUS_INIT();
  const QString feat_str("RedundantProtocol");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetRedundantProtocol(device, "MxL2Redundancy", method.GetProtocols()[restful_str_],
                                                        result_redundant_protocol);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetLocation(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            QString &result_device_location) {
  ACT_STATUS_INIT();
  const QString feat_str("Location");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetDeviceLocation(device, "DeviceLocation", method.GetProtocols()[restful_str_],
                                                     result_device_location);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetDescription(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               QString &result_device_description) {
  ACT_STATUS_INIT();
  const QString feat_str("Description");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetDeviceDescription(device, "DeviceDescription",
                                                        method.GetProtocols()[restful_str_], result_device_description);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetContactInformation(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      QString &result_contact_info) {
  ACT_STATUS_INIT();
  const QString feat_str("ContactInformation");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetContactInformation(device, "ContactInformation",
                                                         method.GetProtocols()[restful_str_], result_contact_info);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetModularInfo(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActDeviceModularInfo &result_modular_info) {
  ACT_STATUS_INIT();
  const QString feat_str("ModularInfo");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetModularInfo(device, "Modules", method.GetProtocols()[restful_str_], result_modular_info);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_modular_info.GetEthernet().isEmpty() && result_modular_info.GetPower().isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

// ACT_STATUS ActSouthbound::ActionGetSystemInformation(const ActDevice &device, const ActFeatureSubItem
// &feat_sub_item,
//                                                      ActMonitorSystemInformation &result_system_information) {
//   ACT_STATUS_INIT();
//   const QString feat_str("SystemInformation");

//   if (feat_sub_item.GetMethods().isEmpty()) {
//     return MethodsEmptyErrorHandler(__func__, device, feat_str);
//   }

//   // Get Method sequence
//   QList<QString> method_sequence;
//   act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
//   if (!IsActStatusSuccess(act_status)) {  // failed
//     return act_status;
//   }

//   for (auto method_key : method_sequence) {
//     auto method = feat_sub_item.GetMethods()[method_key];
//     // Check method protocols status
//     act_status = CheckMethodProtocolsStatus(device, method);
//     if (!IsActStatusSuccess(act_status)) {  // failed
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Start access
//     if (method_key == "Method1") {
//       // RESTful
//       ActRestfulClientHandler restful_handler;
//       act_status = restful_handler.GetSystemInformation(device, "SystemInformation",
//                                                         method.GetProtocols()[restful_str_],
//                                                         result_system_information);
//       if (!IsActStatusSuccess(act_status)) {  // failed
//         qDebug() << __func__ << act_status->GetErrorMessage();
//         continue;
//       }
//     } else if (method_key == "Method2") {
//       // SNMP
//       ActSnmpHandler snmp_handler;
//       qint64 int_value = 0;
//       act_status = snmp_handler.GetIntValue(device, "SysUpTime", method.GetProtocols()[snmp_str_], int_value);
//       if (!IsActStatusSuccess(act_status)) {  // failed
//         qDebug() << __func__ << act_status->GetErrorMessage();
//         continue;
//       } else {
//         result_system_information.SetSystemUptime(FormatUptime(int_value));
//       }
//       // Handle Data
//     } else {
//       // Not match any implemented method
//       act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Validate value
//     if (result_system_information.GetSystemUptime().isEmpty()) {
//       act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }
//     return ACT_STATUS_SUCCESS;
//   }

//   return AccessFailErrorHandler(__func__, device, feat_str, act_status);
// }

ACT_STATUS ActSouthbound::ActionGetSystemUtilization(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     ActMonitorSystemUtilization &result_system_utilization) {
  ACT_STATUS_INIT();
  const QString feat_str("SystemUtilization");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetSystemUtilization(device, "SystemUtilization",
                                                        method.GetProtocols()[restful_str_], result_system_utilization);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      qint64 int_value = -1;
      ActSnmpHandler snmp_handler;
      auto act_status_1 =
          snmp_handler.GetIntValue(device, "UsStatCPUUtilization", method.GetProtocols()[snmp_str_], int_value);
      if (!IsActStatusSuccess(act_status_1)) {
        qDebug() << __func__ << act_status_1->GetErrorMessage();
      } else {
        result_system_utilization.SetCPUUsage(int_value);
      }
      auto act_status_2 =
          snmp_handler.GetIntValue(device, "UsStatMemoryUtilization", method.GetProtocols()[snmp_str_], int_value);
      if (!IsActStatusSuccess(act_status_2)) {
        qDebug() << __func__ << act_status_2->GetErrorMessage();
      } else {
        result_system_utilization.SetMemoryUsage(int_value);
      }

      if (!IsActStatusSuccess(act_status_1) && !IsActStatusSuccess(act_status_2)) {  // failed
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_system_utilization.GetCPUUsage() == qreal(0) && result_system_utilization.GetMemoryUsage() == qreal(0)) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetPortInfo(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            QMap<qint64, ActDevicePortInfoEntry> &result_port_info_map) {
  ACT_STATUS_INIT();
  const QString feat_str("PortInfo");
  result_port_info_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetPortInfo(device, "PortInfo", method.GetProtocols()[restful_str_], result_port_info_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_info_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetPortStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              QMap<qint64, ActMonitorPortStatusEntry> &result_port_status_map) {
  ACT_STATUS_INIT();
  const QString feat_str("PortStatus");
  result_port_status_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetPortStatus(device, "PortStatus", method.GetProtocols()[restful_str_],
                                                 result_port_status_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      QMap<qint64, qint64> south_port_int_map;
      act_status =
          snmp_handler.GetPortIntMap(device, "IfOperStatus", method.GetProtocols()[snmp_str_], south_port_int_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_int_map.keys()) {
        result_port_status_map[port].SetLinkStatus(static_cast<ActLinkStatusTypeEnum>(south_port_int_map[port]));
        if (!kActLinkStatusTypeEnumMap.values().contains(result_port_status_map[port].GetLinkStatus())) {
          result_port_status_map[port].SetLinkStatus(ActLinkStatusTypeEnum::kDown);
        }
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_status_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetFiberCheck(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              QMap<qint64, ActMonitorFiberCheckEntry> &result_port_fiber_map) {
  ACT_STATUS_INIT();
  const QString feat_str("FiberCheck");
  result_port_fiber_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetFiberCheckStatus(device, "FiberCheckStatus", method.GetProtocols()[restful_str_],
                                                       result_port_fiber_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortFiberCheckMap(device, method.GetProtocols()[snmp_str_], result_port_fiber_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_fiber_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTxTotalOctets(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                 QMap<qint64, quint64> &result_port_tx_total_octets_map) {
  ACT_STATUS_INIT();
  const QString feat_str("TxTotalOctets");
  result_port_tx_total_octets_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      QMap<qint64, ActMonitorTrafficStatisticsEntry> south_port_traffic_map;
      act_status = restful_handler.GetTrafficStatistics(device, "TrafficStatistics",
                                                        method.GetProtocols()[restful_str_], south_port_traffic_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_traffic_map.keys()) {
        result_port_tx_total_octets_map[port] = south_port_traffic_map[port].GetTxTotalOctets();
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      QMap<qint64, qint64> south_port_int_map;
      act_status =
          snmp_handler.GetPortIntMap(device, "IfOutOctets", method.GetProtocols()[snmp_str_], south_port_int_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_int_map.keys()) {
        result_port_tx_total_octets_map[port] = static_cast<quint32>(south_port_int_map[port]);
      }
    } else if (method_key == "Method3") {
      // SNMP
      ActSnmpHandler snmp_handler;
      QMap<qint64, quint64> south_port_uint_map;
      act_status =
          snmp_handler.GetPortUintMap(device, "IfHCOutOctets", method.GetProtocols()[snmp_str_], south_port_uint_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_uint_map.keys()) {
        result_port_tx_total_octets_map[port] = static_cast<quint64>(south_port_uint_map[port]);
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_tx_total_octets_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTxTotalPackets(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  QMap<qint64, quint64> &result_port_tx_total_packets_map) {
  ACT_STATUS_INIT();
  const QString feat_str("TxTotalPackets");
  result_port_tx_total_packets_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      QMap<qint64, ActMonitorTrafficStatisticsEntry> south_port_traffic_map;
      act_status = restful_handler.GetTrafficStatistics(device, "TrafficStatistics",
                                                        method.GetProtocols()[restful_str_], south_port_traffic_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_traffic_map.keys()) {
        result_port_tx_total_packets_map[port] = south_port_traffic_map[port].GetTxTotalPackets();
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetIfOutPktsMap(device, method.GetProtocols()[snmp_str_], result_port_tx_total_packets_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method3") {
      // SNMP
      ActSnmpHandler snmp_handler;
      QMap<qint64, qint64> south_port_int_map;
      act_status = snmp_handler.GetPortIntMap(device, "TcstStatTxTotalPackets", method.GetProtocols()[snmp_str_],
                                              south_port_int_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_int_map.keys()) {
        result_port_tx_total_packets_map[port] = static_cast<quint32>(south_port_int_map[port]);
      }
    } else if (method_key == "Method4") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetIfHCOutPktsMap(device, method.GetProtocols()[snmp_str_], result_port_tx_total_packets_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_tx_total_packets_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTrafficUtilization(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      QMap<qint64, qreal> &result_port_traffic_utilization_map) {
  ACT_STATUS_INIT();
  const QString feat_str("TrafficUtilization");
  result_port_traffic_utilization_map.clear();

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      QMap<qint64, ActMonitorTrafficStatisticsEntry> south_port_traffic_map;
      act_status = restful_handler.GetTrafficStatistics(device, "TrafficStatistics",
                                                        method.GetProtocols()[restful_str_], south_port_traffic_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
      // Handle Data
      for (auto port : south_port_traffic_map.keys()) {
        result_port_traffic_utilization_map[port] = south_port_traffic_map[port].GetTrafficUtilization();
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_port_traffic_utilization_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGet1588TimeSyncStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      ActMonitorTimeSyncStatus &result_time_sync_status) {
  ACT_STATUS_INIT();
  const QString feat_str("IEEE1588_2008");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.Get1588DefaultInfo(device, "1588DefaultInfo", method.GetProtocols()[restful_str_],
                                                      result_time_sync_status);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPTPBaseClock(device, method.GetProtocols()[snmp_str_], result_time_sync_status);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_time_sync_status.GetOffsetFromMaster().isEmpty()) {  // empty is failed
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetDot1ASTimeSyncStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        ActMonitorTimeSyncStatus &result_time_sync_status) {
  ACT_STATUS_INIT();
  const QString feat_str("IEEE802Dot1AS_2011");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetDot1asInfo(device, "Dot1asInfo", method.GetProtocols()[restful_str_],
                                                 result_time_sync_status);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetIEEE8021AS(device, method.GetProtocols()[snmp_str_], result_time_sync_status);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_time_sync_status.GetOffsetFromMaster().isEmpty()) {  // empty is failed
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetRstpStatus(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActMonitorRstpStatus &result_rstp_status) {
  ACT_STATUS_INIT();
  const QString feat_str("RSTP status");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Start access
    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status =
          restful_handler.GetRstpStatus(device, "RSTPStatus", method.GetProtocols()[restful_str_], result_rstp_status);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    // Validate value
    if (result_rstp_status.GetPortStatus().isEmpty()) {  // empty is failed
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }
    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTimeSyncBaseConfig(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      ActTimeSyncBaseConfig &result_base_config) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSync Method(Enable & Profile)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetTimeSyncBaseConfig(device, "MxPtp", method.GetProtocols()[restful_str_],
                                                         result_base_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTimeSync802Dot1ASConfig(const ActDevice &device,
                                                           const ActFeatureSubItem &feat_sub_item,
                                                           ActTimeSync802Dot1ASConfig &resutl_time_sync_config) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSync Setting(802.1AS-2011)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetTimeSync802Dot1ASConfig(device, "StDot1as", method.GetProtocols()[restful_str_],
                                                              resutl_time_sync_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTimeSync1588Config(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      ActTimeSync1588Config &resutl_time_sync_config) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSync Setting(1588-2008)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetTimeSync1588Config(device, "Mx1588Default", method.GetProtocols()[restful_str_],
                                                         resutl_time_sync_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTimeSyncIec61850Config(const ActDevice &device,
                                                          const ActFeatureSubItem &feat_sub_item,
                                                          ActTimeSyncIec61850Config &resutl_time_sync_config) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSync Setting(IEC61850-2016)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetTimeSyncIec61850Config(
          device, "Mx1588Iec61850", method.GetProtocols()[restful_str_], resutl_time_sync_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

ACT_STATUS ActSouthbound::ActionGetTimeSyncC37Dot238Config(const ActDevice &device,
                                                           const ActFeatureSubItem &feat_sub_item,
                                                           ActTimeSyncC37Dot238Config &resutl_time_sync_config) {
  ACT_STATUS_INIT();
  const QString feat_str("TimeSync Setting(C37.238-2017)");

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    if (method_key == "Method1") {
      // RESTful
      ActRestfulClientHandler restful_handler;
      act_status = restful_handler.GetTimeSyncC37Dot238Config(
          device, "Mx1588C37238", method.GetProtocols()[restful_str_], resutl_time_sync_config);
      if (!IsActStatusSuccess(act_status)) {  // failed
        qDebug() << __func__ << act_status->GetErrorMessage();
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      qDebug() << __func__ << act_status->GetErrorMessage();
      continue;
    }

    return ACT_STATUS_SUCCESS;
  }

  return AccessFailErrorHandler(__func__, device, feat_str, act_status);
}

// ACT_STATUS ActSouthbound::ActionSetSpanningTree(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
//                                                 const ActRstpTable &rstp_table) {
//   ACT_STATUS_INIT();
//   const QString feat_str("SpanningTree(STP/RSTP)");

//   if (feat_sub_item.GetMethods().isEmpty()) {
//     return MethodsEmptyErrorHandler(__func__, device, feat_str);
//   }

//   // Get Method sequence
//   QList<QString> method_sequence;
//   act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
//   if (!IsActStatusSuccess(act_status)) {  // failed
//     return act_status;
//   }

//   for (auto method_key : method_sequence) {
//     auto method = feat_sub_item.GetMethods()[method_key];
//     // Check method protocols status
//     act_status = CheckMethodProtocolsStatus(device, method);
//     if (!IsActStatusSuccess(act_status)) {  // failed
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     // Start access
//     if (method_key == "Method1") {
//       // SNMP
//       ActSnmpHandler snmp_handler;
//       act_status = snmp_handler.SetSpanningTree(device, method.GetProtocols()[snmp_str_], rstp_table);
//       if (!IsActStatusSuccess(act_status)) {  // failed
//         qDebug() << __func__ << act_status->GetErrorMessage();
//         continue;
//       }
//     } else {
//       // Not match any implemented method
//       act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
//       qDebug() << __func__ << act_status->GetErrorMessage();
//       continue;
//     }

//     return ACT_STATUS_SUCCESS;
//   }

//   return AccessFailErrorHandler(__func__, device, feat_str, act_status);
// }
