#include <QtEndian>

#include "act_new_moxa_command_handler.h"
#include "act_restful_client_handler.h"
#include "act_snmp_handler.h"
#include "act_southbound.hpp"
#include "act_system.hpp"
#include "act_utilities.hpp"

ACT_STATUS ActSouthbound::ProbeActionEnableSnmpService(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                       ActFeatureSubItem &result_feat_sub_item,
                                                       ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("EnableSnmpService");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // Method1 use the RESTful protocol

      // Check protocol element
      if (!method.GetProtocols().contains(restful_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, restful_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // RESTful only check connect status(previous already checked status)

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionModelName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActFeatureSubItem &result_feat_sub_item,
                                               ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("ModelName");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // Probe each method
  QString result_model_name;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpLocSysDesc", method.GetProtocols()[snmp_str_], result_model_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "SysDescr", method.GetProtocols()[snmp_str_], result_model_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_model_name.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionFirmwareVersion(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     ActFeatureSubItem &result_feat_sub_item,
                                                     ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("FirmwareVersion");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QString result_firmware_version;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "SiStatProductInfoFirmwareVersion",
                                            method.GetProtocols()[snmp_str_], result_firmware_version);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "IFwVersion", method.GetProtocols()[snmp_str_], result_firmware_version);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method3") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "FirmwareVersion", method.GetProtocols()[snmp_str_],
                                            result_firmware_version);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_firmware_version.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionDeviceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                ActFeatureSubItem &result_feat_sub_item,
                                                ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("DeviceName");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }
  // Probe each method
  QString result_device_name;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetStrValue(device, "SysName", method.GetProtocols()[snmp_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpLocSysName", method.GetProtocols()[snmp_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method3") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetStrValue(device, "LldpV2LocSysName", method.GetProtocols()[snmp_str_], result_device_name);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_device_name.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionVendorId(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActFeatureSubItem &result_feat_sub_item,
                                              ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("VendorId");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }
  // For each method
  quint32 result_vendor_id = 0;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetSysObjectId(device, "SysObjectID", method.GetProtocols()[snmp_str_], result_vendor_id);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_vendor_id == 0) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionMacTable(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActFeatureSubItem &result_feat_sub_item,
                                              ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("MACTable");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QMap<qint64, QSet<QString>> port_macs_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortMacsMap(device, "Dot1qTpFdbPort", method.GetProtocols()[snmp_str_], port_macs_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (port_macs_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionInterfaceName(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   ActFeatureSubItem &result_feat_sub_item,
                                                   ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("InterfaceName");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }
  // Probe each method
  QMap<qint64, QString> result_if_name_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortStrMap(device, "LldpLocPortId", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortStrMap(device, "LldpV2LocPortId", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method3") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortStrMap(device, "IfName", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method4") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortStrMap(device, "IfDescr", method.GetProtocols()[snmp_str_], result_if_name_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_if_name_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionInterfaceMac(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  ActFeatureSubItem &result_feat_sub_item,
                                                  ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("InterfaceMAC");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }
  // Probe each method
  QMap<qint64, QString> result_if_mac_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortMacMap(device, "IfPhysAddress", method.GetProtocols()[snmp_str_], result_if_mac_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_if_mac_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionPortSpeed(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActFeatureSubItem &result_feat_sub_item,
                                               ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("PortSpeed");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }
  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // Probe each method
  QMap<qint64, qint64> result_port_speed_map;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortIntMap(device, "IfHighSpeed", method.GetProtocols()[snmp_str_], result_port_speed_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetPortIntMap(device, "IfSpeed", method.GetProtocols()[snmp_str_], result_port_speed_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_port_speed_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionLLDP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                          ActFeatureSubItem &result_feat_sub_item,
                                          ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("LLDP(ScanLinks)");

  QMap<qint64, QString> rem_port_mac_map;
  QMap<qint64, QString> rem_port_id_map;
  QMap<qint64, qint64> rem_port_id_subtype_map;

  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // Probe each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetLldpRemPortMacMap(device, "LldpRemChassisId", method.GetProtocols()[snmp_str_],
                                                     rem_port_mac_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortIntMap(device, "LldpRemPortIdSubtype", method.GetProtocols()[snmp_str_],
                                                     rem_port_id_subtype_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      act_status =
          snmp_handler.GetLldpRemPortStrMap(device, "LldpRemPortId", method.GetProtocols()[snmp_str_], rem_port_id_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else if (method_key == "Method2") {
      // SNMP

      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetLldpRemPortMacMap(device, "LldpV2RemChassisId", method.GetProtocols()[snmp_str_],
                                                     rem_port_mac_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortIntMap(device, "LldpV2RemPortIdSubtype", method.GetProtocols()[snmp_str_],
                                                     rem_port_id_subtype_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      act_status = snmp_handler.GetLldpRemPortStrMap(device, "LldpV2RemPortId", method.GetProtocols()[snmp_str_],
                                                     rem_port_id_map);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (rem_port_mac_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemChassisID");
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }
    if (rem_port_id_subtype_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemPortIdSubtype");
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }
    if (rem_port_id_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, "RemPortId");
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionCheckSnmpConnect(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                      ActFeatureSubItem &result_feat_sub_item,
                                                      ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("SNMP connect");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];

    // Start access
    if (method_key == "Method1") {
      // SNMP

      ActSnmpHandler snmp_handler;
      quint32 vendor_id = 0;
      act_status = snmp_handler.GetSysObjectId(device, "SysObjectID", method.GetProtocols()[snmp_str_], vendor_id);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionCheckRestfulConnect(const ActDevice &device,
                                                         const ActFeatureSubItem &feat_sub_item,
                                                         ActFeatureSubItem &result_feat_sub_item,
                                                         ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("RESTful connect");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];

    // Start access
    if (method_key == "Method1") {
      // Method1 use the RESTful protocol

      // Check protocol element
      if (!method.GetProtocols().contains(restful_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, restful_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // RESTful only check connect status(previous already checked status)
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionReboot(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                            ActFeatureSubItem &result_feat_sub_item,
                                            ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("Reboot");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // Method1 use the RESTful protocol

      // Check protocol element
      if (!method.GetProtocols().contains(restful_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, restful_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // RESTful only check connect status(previous already checked status)

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionFactoryDefault(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    ActFeatureSubItem &result_feat_sub_item,
                                                    ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("FactoryDefault");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // Method1 use the RESTful protocol

      // Check protocol element
      if (!method.GetProtocols().contains(restful_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, restful_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // RESTful only check connect status(previous already checked status)

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionFirmwareUpgrade(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     ActFeatureSubItem &result_feat_sub_item,
                                                     ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("FirmwareUpgrade");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }
  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // Method1 use the MoxaIEI-MoxaCommand protocol

      // Check protocol element
      if (!method.GetProtocols().contains(moxa_command_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, moxa_command_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // MoxaCommand only check connect status(previous already checked status)

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionBroadcastSearch(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     ActFeatureSubItem &result_feat_sub_item,
                                                     ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("BroadcastSearch");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }
  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // Method1 use the MoxaIEI-MoxaCommand protocol

      // Check protocol element
      if (!method.GetProtocols().contains(moxa_command_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, moxa_command_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // MoxaIEI-MoxaCommand only check connect status(previous already checked status)

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionNetworkSetting(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    ActFeatureSubItem &result_feat_sub_item,
                                                    ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("NetworkSetting");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // Method1 use the MoxaIEI-MoxaCommand protocol

      // Check protocol element
      if (!method.GetProtocols().contains(moxa_command_str_)) {
        act_status = NotFoundProtocolErrorHandler(__func__, device, feat_str, method_key, moxa_command_str_);
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // MoxaCommand only check connect status(previous already checked status)

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionVLAN(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                          ActFeatureSubItem &result_feat_sub_item,
                                          ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("VLAN Method(VLAN static)");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActVlanStaticEntry> vlan_static_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    if (method_key == "Method2") {
      // SNMP Q-BRIDGE-MIB
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetDot1qVlanStatic(device, method.GetProtocols()[snmp_str_], vlan_static_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionVLANPortType(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                  ActFeatureSubItem &result_feat_sub_item,
                                                  ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("VLANPortType(Access/Trunk/Hybrid)");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActVlanPortTypeEntry> vlan_port_type_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetVlanPortType(device, "VlanConfigVlanPortType", method.GetProtocols()[snmp_str_],
                                                vlan_port_type_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (vlan_port_type_entries.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionTEMSTID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                             ActFeatureSubItem &result_feat_sub_item,
                                             ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("TE-MSTID");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActStaticForwardEntry> static_forward_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionPortPVID(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                              ActFeatureSubItem &result_feat_sub_item,
                                              ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("PortPVID");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActPortVlanEntry> port_vlan_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortPVID(device, "Dot1qPvid", method.GetProtocols()[snmp_str_], port_vlan_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (port_vlan_entries.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionPortDefaultPCP(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    ActFeatureSubItem &result_feat_sub_item,
                                                    ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("PortDefaultPCP");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActDefaultPriorityEntry> default_priority_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortPCP(device, "QosConfigDefaultPriorityValue", method.GetProtocols()[snmp_str_],
                                           default_priority_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (default_priority_entries.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionStreamPriority(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                    ActFeatureSubItem &result_feat_sub_item,
                                                    ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("PerStreamPriority");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActInterfaceStadPortEntry> interface_stad_port_entries;
  QSet<ActStadConfigEntry> stad_config_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;

      // // Ingress (For enhance performance so skip it)
      act_status =
          snmp_handler.GetInterfaceStadPort(device, method.GetProtocols()[snmp_str_], interface_stad_port_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

      // Egress (only probe it)
      act_status = snmp_handler.GetStadConfig(device, method.GetProtocols()[snmp_str_], stad_config_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (stad_config_entries.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionStaticUnicast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                   ActFeatureSubItem &result_feat_sub_item,
                                                   ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("StaticForward(Unicast)");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActStaticForwardEntry> static_forward_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetDot1qStaticUnicast(device, method.GetProtocols()[snmp_str_], static_forward_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionStaticMulticast(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                     ActFeatureSubItem &result_feat_sub_item,
                                                     ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("StaticForward(Multicast)");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActStaticForwardEntry> static_forward_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetDot1qStaticMulticast(device, method.GetProtocols()[snmp_str_], static_forward_entries);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }

    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionSpanningTreeMethod(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                        ActFeatureSubItem &result_feat_sub_item,
                                                        ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree Method(RSTP Method)");

  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  ActRstpTable rstp_table;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetSpanningTreeMethod(device, method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetTWSSpanningTreeMethod(device, method.GetProtocols()[snmp_str_], rstp_table);
      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionSpanningTreeHelloTime(const ActDevice &device,
                                                           const ActFeatureSubItem &feat_sub_item,
                                                           ActFeatureSubItem &result_feat_sub_item,
                                                           ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(HelloTime)");

  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  ActRstpTable rstp_table;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    qint64 int_value = -1;
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetIntValue(device, "Ieee8021SpanningTreeBridgeHelloTime",
                                            method.GetProtocols()[snmp_str_], int_value);

      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetIntValue(device, "SpanningTreeHelloTime", method.GetProtocols()[snmp_str_], int_value);

      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (int_value == -1) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionSpanningTreePriority(const ActDevice &device,
                                                          const ActFeatureSubItem &feat_sub_item,
                                                          ActFeatureSubItem &result_feat_sub_item,
                                                          ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(Priority)");

  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  ActRstpTable rstp_table;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    qint64 int_value = -1;
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetIntValue(device, "Ieee8021SpanningTreePriority", method.GetProtocols()[snmp_str_], int_value);

      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else if (method_key == "Method2") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status =
          snmp_handler.GetIntValue(device, "SpanningTreePriority", method.GetProtocols()[snmp_str_], int_value);

      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (int_value == -1) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeActionSpanningTreeRootGuard(const ActDevice &device,
                                                           const ActFeatureSubItem &feat_sub_item,
                                                           ActFeatureSubItem &result_feat_sub_item,
                                                           ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("SpanningTree(RootGuard)");

  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  ActRstpTable rstp_table;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    // Start access method
    QMap<qint64, qint64> result_port_root_guard_map;
    if (method_key == "Method1") {
      // SNMP
      ActSnmpHandler snmp_handler;
      act_status = snmp_handler.GetPortIntMap(device, "RstpConfigPortRootGuard", method.GetProtocols()[snmp_str_],
                                              result_port_root_guard_map);

      if (!IsActStatusSuccess(act_status)) {  // failed
        warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
        continue;
      }
    } else {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Validate value
    if (result_port_root_guard_map.isEmpty()) {
      act_status = GetDataEmptyFailErrorHandler(__func__, device, feat_str);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeAction802Dot1Qbv(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                                ActFeatureSubItem &result_feat_sub_item,
                                                ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("802.1Qbv(GCL)");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActStaticForwardEntry> static_forward_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}

ACT_STATUS ActSouthbound::ProbeAction802Dot1CB(const ActDevice &device, const ActFeatureSubItem &feat_sub_item,
                                               ActFeatureSubItem &result_feat_sub_item,
                                               ActFeatureSubItemWarning &result_warning) {
  ACT_STATUS_INIT();
  const QString feat_str("802.1CB(FRER & StreamIdentification)");
  result_feat_sub_item.GetMethods().clear();
  ActFeatureSubItemWarning warning;

  if (feat_sub_item.GetMethods().isEmpty()) {
    return MethodsEmptyErrorHandler(__func__, device, feat_str);
  }

  // Get Method sequence
  QList<QString> method_sequence;
  act_status = GetMethodSequence(feat_str, feat_sub_item, method_sequence);
  if (!IsActStatusSuccess(act_status)) {  // failed
    return act_status;
  }

  // For each method
  QSet<ActStaticForwardEntry> static_forward_entries;
  for (auto method_key : method_sequence) {
    auto method = feat_sub_item.GetMethods()[method_key];
    // Check method protocols status
    act_status = CheckMethodProtocolsStatus(device, method);
    if (!IsActStatusSuccess(act_status)) {  // failed
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // If has cache would return this method
    if (CheckMethodHasCache(method)) {
      result_feat_sub_item.GetMethods()[method_key] = method;
      return ACT_STATUS_SUCCESS;
    }

    {
      // Not match any implemented method
      act_status = MethodNotImplementedErrorHandler(__func__, device, feat_str, method_key);
      warning.GetMethods()[method_key] = ActFeatureMethodWarning(act_status->GetErrorMessage());
      continue;
    }

    // Success probe the method
    // Set method to result
    result_feat_sub_item.GetMethods()[method_key] = method;
    // Insert method to cache
    InsertMethodToCache(method);

    return ACT_STATUS_SUCCESS;
  }

  // Can not access would assign result_warnings
  result_warning = warning;
  return ProbeFailErrorHandler(__func__, device, feat_str);
}
