#include "act_snmp_handler.h"

#include <QDebug>
#include <QString>
#include <iostream>
#include <sstream>

#include "act_snmp_result.hpp"
#include "act_snmpset.h"
#include "act_snmpwalk.h"
#include "act_system.hpp"
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h"

ACT_STATUS ActSnmpHandler::NotFoundReturnMessageErrorHandler(const QString &called_func, const QString &item,
                                                             const QString &oid, const ActDevice &device) {
  qCritical() << QString(called_func).toStdString().c_str()
              << QString("Not found SNMP return message(Item: %1, Path: %2). Device: %3")
                     .arg(item)
                     .arg(oid)
                     .arg(device.GetIpv4().GetIpAddress())
                     .toStdString()
                     .c_str();

  return std::make_shared<ActStatusNotFound>(QString("SNMP return message(Item: %1, Path: %2)").arg(item).arg(oid));
}

ACT_STATUS ActSnmpHandler::GetSnmpSubTreeErrorHandler(const QString &called_func, ACT_STATUS status,
                                                      const QString &item, const QString &oid,
                                                      const ActDevice &device) {
  qCritical() << QString(called_func).toStdString().c_str()
              << QString("GetSnmpSubTree(%1(%2)) failed. Device: %3")
                     .arg(item)
                     .arg(oid)
                     .arg(device.GetIpv4().GetIpAddress())
                     .toStdString()
                     .c_str();

  // return status;
  return std::make_shared<ActBadRequest>(QString("SNMP access %1(%2) failed.").arg(item).arg(oid));
}

//////// Refactor start

// ref: https://net-snmp.sourceforge.io/docs/README.thread.html
// Restrictions on Multi-threaded Use of the SNMP Library
// 1. Invoke SOCK_STARTUP or SOCK_CLEANUP from the main thread only.
ACT_STATUS ActSnmpHandler::InitSnmpResource() {
  ACT_STATUS_INIT();
  try {
    SOCK_STARTUP;
    init_snmp("chamberlain_snmp");

  } catch (std::exception &e) {
    qCritical() << "ActSnmpAgent::InitSNMP() failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::ClearSnmpResource() {
  ACT_STATUS_INIT();
  try {
    SOCK_CLEANUP;

  } catch (std::exception &e) {
    qCritical() << "ActSnmpAgent::ClearSnmpResource() failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetStrValue(const ActDevice &device, const QString &action_key,
                                       const ActFeatureMethodProtocol &protocol_elem, QString &str_value) {
  ACT_STATUS_INIT();
  str_value = "";
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Find return message
  QString key = action.GetPath() + ".0";
  auto msg_it = snmp_message_result_map.find(key);
  if (msg_it == snmp_message_result_map.end()) {
    key = action.GetPath() + ".1";
    msg_it = snmp_message_result_map.find(key);
    if (msg_it == snmp_message_result_map.end()) {
      return NotFoundReturnMessageErrorHandler(__func__, action_key, action.GetPath(), device);
    }
  }
  str_value = msg_it.value();
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetIntValue(const ActDevice &device, const QString &action_key,
                                       const ActFeatureMethodProtocol &protocol_elem, qint64 &int_value) {
  ACT_STATUS_INIT();
  int_value = -1;
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Find return message
  QString key = action.GetPath() + ".0";
  auto msg_it = snmp_message_result_map.find(key);
  if (msg_it == snmp_message_result_map.end()) {
    key = action.GetPath() + ".1";
    msg_it = snmp_message_result_map.find(key);
    if (msg_it == snmp_message_result_map.end()) {
      return NotFoundReturnMessageErrorHandler(__func__, action_key, action.GetPath(), device);
    }
  }
  int_value = msg_it.value().toInt();
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetSysObjectId(const ActDevice &device, const QString &action_key,
                                          const ActFeatureMethodProtocol &protocol_elem, quint32 &sys_object_id) {
  ACT_STATUS_INIT();
  sys_object_id = 0;
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Find return message
  QString key = action.GetPath() + ".0";
  auto msg_it = snmp_message_result_map.find(key);
  if (msg_it == snmp_message_result_map.end()) {
    return NotFoundReturnMessageErrorHandler(__func__, action_key, key, device);
  }

  // ".1.3.6.1.4.1.8691.600.1.4.2" -> "8691"
  // or "iso.3.6.1.4.1.8691.600.1.4.2" -> "8691"
  // or enterprises.8691.600.1.4.1

  QString sys_object_id_str = "";
  if (msg_it.value().contains("enterprises.")) {
    sys_object_id_str = msg_it.value().split("enterprises.").at(1).split(".").at(0);
  } else {
    sys_object_id_str = msg_it.value().split("3.6.1.4.1.").at(1).split(".").at(0);
  }

  sys_object_id = sys_object_id_str.toUInt();
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetLldpRemPortMacMap(const ActDevice &device, const QString &action_key,
                                                const ActFeatureMethodProtocol &protocol_elem,
                                                QMap<qint64, QString> &port_mac_map) {
  ACT_STATUS_INIT();
  port_mac_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port remote MAC map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Get port
    qint64 port;
    LldpRemOidToPort(action.GetPath(), map_it.key(), port);

    // "00 90 E8 11 22 44" -> "00-90-E8-11-22-44"
    QString mac_address = map_it.value();
    mac_address = mac_address.replace(" ", "-").toUpper();

    // "00-90-E8-11-22-D" -> "00-90-E8-11-22-0D"
    qint64 mac_int = 0;
    MacAddressToQInt64(mac_address, mac_int);
    QInt64ToMacAddress(mac_int, mac_address);

    port_mac_map.insert(port, mac_address);  // MAC
  }
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetLldpRemPortIntMap(const ActDevice &device, const QString &action_key,
                                                const ActFeatureMethodProtocol &protocol_elem,
                                                QMap<qint64, qint64> &port_int_map) {
  ACT_STATUS_INIT();
  port_int_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Get port
    qint64 port;
    LldpRemOidToPort(action.GetPath(), map_it.key(), port);
    port_int_map.insert(port, map_it.value().toInt());
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetLldpRemPortStrMap(const ActDevice &device, const QString &action_key,
                                                const ActFeatureMethodProtocol &protocol_elem,
                                                QMap<qint64, QString> &port_str_map) {
  ACT_STATUS_INIT();
  port_str_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Get port
    qint64 port;
    LldpRemOidToPort(action.GetPath(), map_it.key(), port);
    port_str_map.insert(port, map_it.value());
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortStrMap(const ActDevice &device, const QString &action_key,
                                         const ActFeatureMethodProtocol &protocol_elem,
                                         QMap<qint64, QString> &port_str_map) {
  ACT_STATUS_INIT();
  port_str_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port String map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    qint64 port = map_it.key().mid(action.GetPath().size() + 1).toInt();
    port_str_map.insert(port, map_it.value());
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortMacMap(const ActDevice &device, const QString &action_key,
                                         const ActFeatureMethodProtocol &protocol_elem,
                                         QMap<qint64, QString> &port_mac_map) {
  ACT_STATUS_INIT();
  port_mac_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port String map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    qint64 port = map_it.key().mid(action.GetPath().size() + 1).toInt();

    // "00 90 E8 11 22 44" -> "00-90-E8-11-22-44"
    QString mac_address = map_it.value();
    mac_address = mac_address.replace(" ", "-").toUpper();

    // "00-90-E8-11-22-D" -> "00-90-E8-11-22-0D"
    qint64 mac_int = 0;
    MacAddressToQInt64(mac_address, mac_int);
    QInt64ToMacAddress(mac_int, mac_address);

    port_mac_map.insert(port, mac_address);
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortIntMap(const ActDevice &device, const QString &action_key,
                                         const ActFeatureMethodProtocol &protocol_elem,
                                         QMap<qint64, qint64> &port_int_map) {
  ACT_STATUS_INIT();
  port_int_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port Int map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    qint64 port = map_it.key().mid(action.GetPath().size() + 1).toInt();

    quint64 value = map_it.value().toInt();
    if (action_key == "IfSpeed") {
      value = value / 1000000;  // Mbps
    }
    port_int_map.insert(port, value);
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortUintMap(const ActDevice &device, const QString &action_key,
                                          const ActFeatureMethodProtocol &protocol_elem,
                                          QMap<qint64, quint64> &port_uint_map) {
  ACT_STATUS_INIT();
  port_uint_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port Int map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    qint64 port = map_it.key().mid(action.GetPath().size() + 1).toInt();
    quint64 value = map_it.value().toULongLong();
    if (action_key == "IfSpeed") {
      value = value / 1000000;  // Mbps
    }
    port_uint_map.insert(port, value);
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortMacsMap(const ActDevice &device, const QString &action_key,
                                          const ActFeatureMethodProtocol &protocol_elem,
                                          QMap<qint64, QSet<QString>> &port_macs_map) {
  ACT_STATUS_INIT();
  port_macs_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Insert Port remote MAC map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Dot1qTpFdbPort(Q-BRIDGE-MIB)
    // request  mib = 1.3.6.1.2.1.17.7.1.2.2.1.2
    // respones mib = 1.3.6.1.2.1.17.7.1.2.2.1.2.1.0.144.232.17.34.13
    // mac 0.144.232.17.34.13
    QString mib_body = map_it.key().mid(action.GetPath().size() + 1);  // "1.0.144.232.17.34.13"
    mib_body = mib_body.section(".", 1);                               // "0.144.232.17.34.13"
    QString mac = MacDecToHex(mib_body);

    qint64 port = map_it.value().toInt();

    if (port_macs_map.contains(port)) {  // map has port set
      port_macs_map[port].insert(mac);
      continue;
    }

    // Map not contain would insert new set
    QSet<QString> mac_set;
    mac_set.insert(mac);
    port_macs_map.insert(port, mac_set);
  }
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortFiberCheckMap(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                QMap<qint64, ActMonitorFiberCheckEntry> &result_port_fiber_map) {
  ACT_STATUS_INIT();
  result_port_fiber_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("FiberCheckStatModelName");
  use_action_str_set.insert("FiberCheckStatSN");
  use_action_str_set.insert("FiberCheckStatWaveLength");
  use_action_str_set.insert("FiberCheckStatVoltage");
  use_action_str_set.insert("FiberCheckStatTemperatureC");
  use_action_str_set.insert("FiberCheckStatTemperatureLimitC");
  use_action_str_set.insert("FiberCheckStatTemperatureF");
  use_action_str_set.insert("FiberCheckStatTemperatureLimitF");
  use_action_str_set.insert("FiberCheckStatTxPower");
  use_action_str_set.insert("FiberCheckStatTxPowerLimit");
  use_action_str_set.insert("FiberCheckStatRxPower");
  use_action_str_set.insert("FiberCheckStatRxPowerLimit");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Get Port
    // request  mib = 1.3.6.1.4.1.8691.603.5.3.2.1.1.1.2
    // respones mib = 1.3.6.1.4.1.8691.603.5.3.2.1.1.1.2.3
    // port 3
    QStringList parts = map_it.key().split(".");
    qint64 port = parts.last().toInt();

    // Get Port entry at map
    ActMonitorFiberCheckEntry fiber_check_entry;
    if (result_port_fiber_map.contains(port)) {
      fiber_check_entry = result_port_fiber_map[port];
    }

    if (map_it.key().contains(action_oid_map["FiberCheckStatModelName"])) {
      fiber_check_entry.SetModelName(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatSN"])) {
      fiber_check_entry.SetSerialNumber(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatWaveLength"])) {
      fiber_check_entry.SetWavelength(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatVoltage"])) {
      fiber_check_entry.SetVoltage(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatTemperatureC"])) {
      fiber_check_entry.SetTemperatureC(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatTemperatureLimitC"])) {
      fiber_check_entry.SetTemperatureLimitC(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatTemperatureF"])) {
      fiber_check_entry.SetTemperatureF(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatTemperatureLimitF"])) {
      fiber_check_entry.SetTemperatureLimitF(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatTxPower"])) {
      fiber_check_entry.SetTxPower(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatTxPowerLimit"])) {
      // '["-15.00", "3.00"]' -> "-15.00", "3.00"
      QString cleaned_input = map_it.value().mid(1, map_it.value().length() - 2);
      QStringList parts = cleaned_input.split(", ");
      fiber_check_entry.SetTxPowerLimit(parts.toVector().toList());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatRxPower"])) {
      fiber_check_entry.SetRxPower(map_it.value());
    } else if (map_it.key().contains(action_oid_map["FiberCheckStatRxPowerLimit"])) {
      // '["-15.00", "3.00"]' -> "-15.00", "3.00"
      QString cleaned_input = map_it.value().mid(1, map_it.value().length() - 2);
      QStringList parts = cleaned_input.split(", ");
      fiber_check_entry.SetRxPowerLimit(parts.toVector().toList());
    } else {
      qCritical() << __func__ << "Reply not defined";
      continue;
    }

    result_port_fiber_map[port] = fiber_check_entry;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetIfOutPktsMap(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                           QMap<qint64, quint64> &result_port_packets_map) {
  ACT_STATUS_INIT();
  result_port_packets_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("IfOutUcastPkts");
  use_action_str_set.insert("IfOutNUcastPkts");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
      act_status =
          SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    } else {  // v2c && v3 support bulk request
      act_status =
          SendRequestAndInsertResultMapByBulk(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    }
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Get Port
    // request  mib = 1.3.6.1.2.1.2.2.1.17
    // respones mib = 1.3.6.1.2.1.2.2.1.17.3
    // port 3
    QStringList parts = map_it.key().split(".");
    qint64 port = parts.last().toInt();

    // Get Port entry at map
    quint64 total_packets = 0;
    if (result_port_packets_map.contains(port)) {
      total_packets = result_port_packets_map[port];
    }

    // tx_total_packets =  Unicast packets + Non Unicast packets(broadcast + multicast)
    if (map_it.key().contains(action_oid_map["IfOutUcastPkts"])) {
      total_packets += map_it.value().toULongLong();
    } else if (map_it.key().contains(action_oid_map["IfOutNUcastPkts"])) {
      total_packets += map_it.value().toULongLong();
    } else {
      qCritical() << __func__ << "Reply not defined";
      continue;
    }

    result_port_packets_map[port] = total_packets;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetIfHCOutPktsMap(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                             QMap<qint64, quint64> &result_port_packets_map) {
  ACT_STATUS_INIT();
  result_port_packets_map.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("IfHCOutUcastPkts");
  use_action_str_set.insert("IfHCOutMulticastPkts");
  use_action_str_set.insert("IfHCOutBroadcastPkts");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
      act_status =
          SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    } else {  // v2c && v3 support bulk request
      act_status =
          SendRequestAndInsertResultMapByBulk(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    }
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    // Get Port
    // request  mib = 1.3.6.1.2.1.31.1.1.1.11
    // respones mib = 1.3.6.1.2.1.31.1.1.1.11.3
    // port 3
    QStringList parts = map_it.key().split(".");
    qint64 port = parts.last().toInt();

    // Get Port entry at map
    quint64 total_packets = 0;
    if (result_port_packets_map.contains(port)) {
      total_packets = result_port_packets_map[port];
    }

    // tx_total_packets =  Unicast packets + Multicast packets + Broadcast packets
    if (map_it.key().contains(action_oid_map["IfHCOutUcastPkts"])) {
      total_packets += map_it.value().toULongLong();
    } else if (map_it.key().contains(action_oid_map["IfHCOutMulticastPkts"])) {
      total_packets += map_it.value().toULongLong();
    } else if (map_it.key().contains(action_oid_map["IfHCOutBroadcastPkts"])) {
      total_packets += map_it.value().toULongLong();
    } else {
      qCritical() << __func__ << "Reply not defined";
      continue;
    }

    result_port_packets_map[port] = total_packets;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPTPBaseClock(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                           ActMonitorTimeSyncStatus &result_time_sync_status) {
  // 1588-2008(v2)

  ACT_STATUS_INIT();
  QMap<QString, QString> snmp_message_result_map;

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("PtpbaseClockParentDSGMClockIdentity");
  use_action_str_set.insert("PtpbaseClockParentDSParentPortIdentity");
  use_action_str_set.insert("PtpbaseClockPortRunningState");
  use_action_str_set.insert("PtpbaseClockCurrentDSOffsetFromMaster");
  use_action_str_set.insert("PtpbaseClockCurrentDSStepsRemoved");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    if (map_it.key().contains(action_oid_map["PtpbaseClockParentDSGMClockIdentity"])) {
      // GrandmasterIdentity
      // "00 90 E8 FF FE 11 22 09" -> "00:90:E8:FF:FE:11:22:09"
      QString value = map_it.value();
      value = value.replace(" ", "-").toUpper();
      result_time_sync_status.SetGrandmasterIdentity(value);
    } else if (map_it.key().contains(action_oid_map["PtpbaseClockParentDSParentPortIdentity"])) {
      // ParentIdentity
      // "00 90 E8 FF FE 11 22 09" -> "00:90:E8:FF:FE:11:22:09"
      QString value = map_it.value();
      value = value.replace(" ", "-").toUpper();
      result_time_sync_status.SetParentIdentity(value);
    } else if (map_it.key().contains(action_oid_map["PtpbaseClockPortRunningState"])) {
      // Port State map
      // request  mib = 1.3.6.1.2.1.241.1.2.9.1.6
      // respones mib = 1.3.6.1.2.1.241.1.2.9.1.6.0.0.1.4
      // port 4
      QStringList parts = map_it.key().split(".");
      qint64 port = parts.last().toInt();

      // value  disabled(3), master(6), slave(9)
      result_time_sync_status.GetPortState()[port] = static_cast<quint8>(map_it.value().toUShort());
    } else if (map_it.key().contains(action_oid_map["PtpbaseClockCurrentDSOffsetFromMaster"])) {
      // OffsetFromMaster
      result_time_sync_status.SetOffsetFromMaster(map_it.value());
    } else if (map_it.key().contains(action_oid_map["PtpbaseClockCurrentDSStepsRemoved"])) {
      // StepsRemoved
      result_time_sync_status.SetStepsRemoved(map_it.value().toULongLong());
    } else {
      qCritical() << __func__ << "Reply not defined";
      continue;
    }
  }

  return act_status;
}

// Function to multiply and shift a 64-bit value by a large power of 2
quint64 multiplyAndShift(quint64 value, int shift) {
  if (shift == 0) {
    return value;
  } else if (shift < 64) {
    return value << shift;
  } else {
    // For shifts >= 64, we need to split the value into high and low parts
    quint64 high = value << (shift - 64);
    return high;
  }
}

ACT_STATUS ActSnmpHandler::GetIEEE8021AS(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                         ActMonitorTimeSyncStatus &result_time_sync_status) {
  // 802.1AS-2011

  ACT_STATUS_INIT();
  QMap<QString, QString> snmp_message_result_map;

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Ieee8021AsParentDSGrandmasterIdentity");
  use_action_str_set.insert("Ieee8021AsParentDSParentClockIdentity");
  use_action_str_set.insert("Ieee8021AsPortDSPortRole");
  use_action_str_set.insert("Ieee8021AsCurrentDSOffsetFromMasterHs");
  use_action_str_set.insert("Ieee8021AsCurrentDSOffsetFromMasterMs");
  use_action_str_set.insert("Ieee8021AsCurrentDSOffsetFromMasterLs");
  use_action_str_set.insert("Ieee8021AsCurrentDSStepsRemoved");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // For OffsetFromMaster
  QString map_value_hs = "";
  QString map_value_ms = "";
  QString map_value_ls = "";

  // Insert Port Data map
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    if (map_it.key().contains(action_oid_map["Ieee8021AsParentDSGrandmasterIdentity"])) {
      // GrandmasterIdentity
      // "00 90 E8 FF FE 11 22 09" -> "00:90:E8:FF:FE:11:22:09"
      QString value = map_it.value();
      value = value.replace(" ", "-").toUpper();
      result_time_sync_status.SetGrandmasterIdentity(value);
    } else if (map_it.key().contains(action_oid_map["Ieee8021AsParentDSParentClockIdentity"])) {
      // ParentIdentity
      // "00 90 E8 FF FE 11 22 09" -> "00:90:E8:FF:FE:11:22:09"
      QString value = map_it.value();
      value = value.replace(" ", "-").toUpper();
      result_time_sync_status.SetParentIdentity(value);
    } else if (map_it.key().contains(action_oid_map["Ieee8021AsPortDSPortRole"])) {
      // Port State map
      // request  mib = .1.3.111.2.802.1.1.20.1.5.1.5
      // respones mib = .1.3.111.2.802.1.1.20.1.5.1.5.4.4
      // port 4
      QStringList parts = map_it.key().split(".");
      qint64 port = parts.last().toInt();
      // value: disabledPort(3), masterPort(6), slavePort(9)
      result_time_sync_status.GetPortState()[port] = static_cast<quint8>(map_it.value().toUShort());
    } else if (map_it.key().contains(action_oid_map["Ieee8021AsCurrentDSOffsetFromMasterHs"])) {
      // OffsetFromMaster(Hs)
      map_value_hs = map_it.value();
    } else if (map_it.key().contains(action_oid_map["Ieee8021AsCurrentDSOffsetFromMasterMs"])) {
      // OffsetFromMaster(Ms)
      map_value_ms = map_it.value();
    } else if (map_it.key().contains(action_oid_map["Ieee8021AsCurrentDSOffsetFromMasterLs"])) {
      // OffsetFromMaster(Ls)
      map_value_ls = map_it.value();
    } else if (map_it.key().contains(action_oid_map["Ieee8021AsCurrentDSStepsRemoved"])) {
      // StepsRemoved
      result_time_sync_status.SetStepsRemoved(map_it.value().toULongLong());
    } else {
      qCritical() << __func__ << "Reply not defined";
      continue;
    }
  }

  // Handle OffsetFromMaster(HsStr + MsStr + LsStr)
  // HS : MS : LS ( 65535 ~ 2^96)
  //  LS: 2^16 ~ 2^32
  //  MS: 2^32 ~ 2^64
  //  HS: 2^64 ~ 2^96

  // Combined value = HS * (2^96) + MS * (2^64) + LS
  // quint64 combined_value = (map_value_hs.toUInt() * static_cast<quint64>(1) << 96) +
  //                          (map_value_ms.toUInt() * static_cast<quint64>(1) << 64) + map_value_ls.toUInt();
  quint64 hs_part = multiplyAndShift(map_value_hs.toUInt(), 96);
  quint64 ms_part = multiplyAndShift(map_value_ms.toUInt(), 64);
  quint64 ls_part = map_value_ls.toUInt();

  // Since hs_part and ms_part are already shifted, we can safely add them
  quint64 combined_value = hs_part + ms_part + ls_part;

  // Divide the combined value by 65536
  quint64 result_value = combined_value / 65536;

  result_time_sync_status.SetOffsetFromMaster(QString::number(result_value));
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetDot1qVlanStatic(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                              QSet<ActVlanStaticEntry> &vlan_static_entries) {
  ACT_STATUS_INIT();

  vlan_static_entries.clear();

  QMap<QString, QString> snmp_message_result_map;

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Dot1qVlanStaticName");
  use_action_str_set.insert("Dot1qVlanStaticEgressPorts");
  use_action_str_set.insert("Dot1qVlanStaticUntaggedPorts");
  use_action_str_set.insert("Dot1qVlanStaticRowStatus");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    // Dot1qVlanStaticName
    // request  mib = 1.3.6.1.2.1.17.7.1.4.3.1.1
    // respones mib = 1.3.6.1.2.1.17.7.1.4.3.1.1.3
    // vid 3

    // Get VID
    QStringList parts = map_it.key().split(".");
    qint64 vid = parts.last().toInt();

    // Find entry by VID
    ActVlanStaticEntry vlan_static_entry(vid);
    auto vlan_entry_found_it = vlan_static_entries.find(vlan_static_entry);
    if (vlan_entry_found_it != vlan_static_entries.end()) {  // found
      vlan_static_entry = *vlan_entry_found_it;
      vlan_static_entries.erase(vlan_entry_found_it);
    }

    // Set entry
    if (map_it.key().contains(action_oid_map["Dot1qVlanStaticName"])) {
      // Name
      vlan_static_entry.SetName(map_it.value());
    } else if (map_it.key().contains(action_oid_map["Dot1qVlanStaticEgressPorts"])) {
      // EgressPorts
      vlan_static_entry.SetEgressPorts(HexToPorts(map_it.value()));
    } else if (map_it.key().contains(action_oid_map["Dot1qVlanStaticUntaggedPorts"])) {
      // UntaggedPorts
      vlan_static_entry.SetUntaggedPorts(HexToPorts(map_it.value()));
    } else if (map_it.key().contains(action_oid_map["Dot1qVlanStaticRowStatus"])) {
      // RowStatus
      vlan_static_entry.SetRowStatus(map_it.value().toInt());
    }
    vlan_static_entries.insert(vlan_static_entry);
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetDot1qVlanStatic(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                              const QSet<ActVlanStaticEntry> &vlan_static_entries) {
  ACT_STATUS_INIT();

  // Create use item's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Dot1qVlanStaticName");
  use_action_str_set.insert("Dot1qVlanStaticEgressPorts");
  use_action_str_set.insert("Dot1qVlanStaticUntaggedPorts");
  use_action_str_set.insert("Dot1qVlanStaticRowStatus");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  for (auto vlan_static_entry : vlan_static_entries) {
    QList<ActSnmpSetEntry> snmp_set_entry_list;

    // Generate SnmpSetEntry List
    QString oid;
    QString value;

    // Dot1qVlanStaticRowStatus
    if (vlan_static_entry.GetRowStatus() == 6) {  // destroy
      oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticRowStatus"]).arg(vlan_static_entry.GetVlanId());
      value = QString::number(vlan_static_entry.GetRowStatus());
      snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
    } else {                                        // create
      if (vlan_static_entry.GetRowStatus() == 1) {  // active(1)
        oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticRowStatus"]).arg(vlan_static_entry.GetVlanId());
        value = "5";  // createAndWait(5)
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
      }

      // Dot1qVlanStaticRowStatus
      if (vlan_static_entry.GetRowStatus() != 0) {  // skip (0)
        oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticRowStatus"]).arg(vlan_static_entry.GetVlanId());
        value = QString::number(vlan_static_entry.GetRowStatus());
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
      }

      // Dot1qVlanStaticName
      oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticName"]).arg(vlan_static_entry.GetVlanId());
      snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, vlan_static_entry.GetName(), 's'));

      ActVlanStaticEntry vlan_static_entry_local(vlan_static_entry);

      // [feat: 529] untag
      if (!device.GetDeviceProperty().GetFeatureGroup().GetConfiguration().GetVLANSetting().GetHybridMode()) {
        // Dot1qVlanStaticEgressPorts
        oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticEgressPorts"]).arg(vlan_static_entry.GetVlanId());
        value = BinToHex(PortsToBin(vlan_static_entry_local.GetEgressPorts()));
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'x'));
      } else {
        // Merge two table
        // The switch must add UntagPorts to EgressPorts and then set them to UntagPorts.
        // Directly set UntagPorts would return error.
        QSet port_set = vlan_static_entry_local.GetEgressPorts();
        foreach (auto port, vlan_static_entry_local.GetUntaggedPorts()) {
          port_set.insert(port);
        }

        // Dot1qVlanStaticEgressPorts
        oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticEgressPorts"]).arg(vlan_static_entry.GetVlanId());
        value = BinToHex(PortsToBin(port_set));
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'x'));

        // Dot1qVlanStaticUntaggedPortsOid
        if (vlan_static_entry_local.GetUntaggedPorts().size() != 0) {
          oid = QString("%1.%2").arg(action_oid_map["Dot1qVlanStaticUntaggedPorts"]).arg(vlan_static_entry.GetVlanId());
          value = BinToHex(PortsToBin(vlan_static_entry_local.GetUntaggedPorts()));
          snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'x'));
        }
      }
    }

    // Send SNMP Set
    act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetSnmp(SetVlanStatic) failed. device:" << device.GetIpv4().GetIpAddress();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler ::GetVlanPortType(const ActDevice &device, const QString &action_key,
                                            const ActFeatureMethodProtocol &protocol_elem,
                                            QSet<ActVlanPortTypeEntry> &vlan_port_type_entries) {
  ACT_STATUS_INIT();

  vlan_port_type_entries.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    QString port = map_it.key().mid(action.GetPath().size() + 1);
    vlan_port_type_entries.insert(ActVlanPortTypeEntry(
        port.toInt(), static_cast<ActVlanPortTypeEnum>(map_it.value().toInt()), ActVlanPriorityEnum::kNonTSN));
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler ::GetPortVlanType(const ActDevice &device, const QString &action_key,
                                            const ActFeatureMethodProtocol &protocol_elem,
                                            QSet<ActVlanPortTypeEntry> &vlan_port_type_entries) {
  ACT_STATUS_INIT();

  vlan_port_type_entries.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    QString port = map_it.key().mid(action.GetPath().size() + 1);
    // Change value: access(0), trunk(1), hybrid(2) -> kAccess = 1, kTrunk = 2, kHybrid = 3
    vlan_port_type_entries.insert(ActVlanPortTypeEntry(
        port.toInt(), static_cast<ActVlanPortTypeEnum>(map_it.value().toInt() + 1), ActVlanPriorityEnum::kNonTSN));
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetVlanPortType(const ActDevice &device, const QString &action_key,
                                           const ActFeatureMethodProtocol &protocol_elem,
                                           const QSet<ActVlanPortTypeEntry> &vlan_port_type_entries) {
  ACT_STATUS_INIT();

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  QList<ActSnmpSetEntry> snmp_set_entry_list;
  // Handle each entry
  for (auto entry : vlan_port_type_entries) {
    QString port_oid = QString::number(entry.GetPortId());
    QString oid = QString("%1.%2").arg(action.GetPath()).arg(port_oid);
    snmp_set_entry_list.push_back(
        ActSnmpSetEntry(oid, QString::number(static_cast<int>(entry.GetVlanPortType())), 'i'));
  }

  // SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("SetSnmp(%1) failed. Device: %2").arg(action_key).arg(device.GetIpv4().GetIpAddress());
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetPortVlanType(const ActDevice &device, const QString &action_key,
                                           const ActFeatureMethodProtocol &protocol_elem,
                                           const QSet<ActVlanPortTypeEntry> &vlan_port_type_entries) {
  ACT_STATUS_INIT();

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  QList<ActSnmpSetEntry> snmp_set_entry_list;
  // Handle each entry
  for (auto entry : vlan_port_type_entries) {
    QString port_oid = QString::number(entry.GetPortId());
    QString oid = QString("%1.%2").arg(action.GetPath()).arg(port_oid);

    // Change value: kAccess = 1, kTrunk = 2, kHybrid = 3 -> access(0), trunk(1), hybrid(2)
    QString value = QString::number(static_cast<int>(entry.GetVlanPortType()) - 1);
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
  }

  // SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("SetSnmp(%1) failed. Device: %2").arg(action_key).arg(device.GetIpv4().GetIpAddress());
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortPVID(const ActDevice &device, const QString &action_key,
                                       const ActFeatureMethodProtocol &protocol_elem,
                                       QSet<ActPortVlanEntry> &port_vlan_entries) {
  ACT_STATUS_INIT();

  port_vlan_entries.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    QString port = map_it.key().mid(action.GetPath().size() + 1);
    port_vlan_entries.insert(
        ActPortVlanEntry(port.toInt(), static_cast<quint16>(map_it.value().toUInt()), ActVlanPriorityEnum::kNonTSN));
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetPortPVID(const ActDevice &device, const QString &action_key,
                                       const ActFeatureMethodProtocol &protocol_elem,
                                       const QSet<ActPortVlanEntry> &port_vlan_entries) {
  ACT_STATUS_INIT();

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Handle each entry
  for (auto entry : port_vlan_entries) {
    QString oid = QString("%1.%2").arg(action.GetPath()).arg(entry.GetPortId());
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, QString::number(entry.GetPVID()), 'i'));
  }

  // SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("SetSnmp(%1) failed. Device: %2").arg(action_key).arg(device.GetIpv4().GetIpAddress());
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetPortPCP(const ActDevice &device, const QString &action_key,
                                      const ActFeatureMethodProtocol &protocol_elem,
                                      QSet<ActDefaultPriorityEntry> &default_priority_entries) {
  ACT_STATUS_INIT();

  default_priority_entries.clear();
  QMap<QString, QString> snmp_message_result_map;

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  if (device.GetSnmpConfiguration().GetVersion() == ActSnmpVersionEnum::kV1) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  } else {  // v2c && v3 support bulk request
    act_status = SendRequestAndInsertResultMapByBulk(device, action_key, action.GetPath(), snmp_message_result_map);
  }
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    QString port = map_it.key().mid(action.GetPath().size() + 1);
    default_priority_entries.insert(
        ActDefaultPriorityEntry(port.toInt(), static_cast<quint16>(map_it.value().toUInt())));
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetPortPCP(const ActDevice &device, const QString &action_key,
                                      const ActFeatureMethodProtocol &protocol_elem,
                                      const QSet<ActDefaultPriorityEntry> &default_priority_entries) {
  ACT_STATUS_INIT();

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Handle each entry
  for (auto entry : default_priority_entries) {
    QString oid = QString("%1.%2").arg(action.GetPath()).arg(entry.GetPortId());
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, QString::number(entry.GetDefaultPCP()), 'i'));
  }

  // SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__
                << QString("SetSnmp(%1) failed. Device: %2").arg(action_key).arg(device.GetIpv4().GetIpAddress());
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetInterfaceStadPort(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                QSet<ActInterfaceStadPortEntry> &interface_stad_port_entries) {
  ACT_STATUS_INIT();

  interface_stad_port_entries.clear();

  QMap<QString, QString> snmp_message_result_map;

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("StadConfigIndexEnable");
  use_action_str_set.insert("StadConfigEthertypeValue");
  use_action_str_set.insert("StadConfigSubtypeEnable");
  use_action_str_set.insert("StadConfigSubtypeValue");
  use_action_str_set.insert("StadConfigVlanId");
  use_action_str_set.insert("StadConfigVlanPCP");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // First Step:
  // Get the Enable stad_port_entry index
  // Send SNMP request
  act_status = SendRequestAndInsertResultMap(device, "StadConfigIndexEnable", action_oid_map["StadConfigIndexEnable"],
                                             snmp_message_result_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Check has response message
  if (snmp_message_result_map.isEmpty()) {
    return NotFoundReturnMessageErrorHandler(__func__, "StadConfigIndexEnable", action_oid_map["StadConfigIndexEnable"],
                                             device);
  }

  // Second Step:
  // Generate the EnableEntry SnmpRequestList by the Enable stad_port_entry index
  QList<QString> request_oid_list;
  QMap<QString, QString> enabled_index_snmp_message_map;
  act_status = StadInedexEnableSnmpMessageMapToSnmpRequestList(action_oid_map, snmp_message_result_map,
                                                               enabled_index_snmp_message_map, request_oid_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "StadInedexEnableSnmpMessageMapToSnmpRequestList() Failed.";
    return act_status;
  }

  // The switch StadPort settings are empty would return.
  if (request_oid_list.isEmpty()) {
    return act_status;
  }

  // Third Step:
  // Send SNMP by request_oid_list
  ActSnmpResult<ActSnmpMessageMap> get_snmp_result(device.GetIpv4().GetIpAddress());
  ActSnmpResult<ActSnmpMessageMap> get_snmp_list_result(device.GetIpv4().GetIpAddress());
  act_status = ActSnmpwalk::GetSnmpList(request_oid_list, device, get_snmp_list_result);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetSnmpList() failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  // Merge (enabled_index_snmp_message_map) & (request_oid_list -> snmp_message_map)
  QMap<QString, QString> snmp_message_map = enabled_index_snmp_message_map;
  QMapIterator<QString, QString> map_it(get_snmp_list_result.GetSnmpMessage());
  while (map_it.hasNext()) {
    map_it.next();
    snmp_message_map.insert(map_it.key(), map_it.value());
  }

  // Generate the interface_stad_port_entries
  act_status = SnmpMessageMapToInterfaceStadPort(action_oid_map, snmp_message_map, interface_stad_port_entries);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "SnmpMessageMapToInterfaceStadPort() Failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetInterfaceStadPort(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                const QSet<ActInterfaceStadPortEntry> &interface_stad_port_entries) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("StadConfigIndexEnable");
  use_action_str_set.insert("StadConfigEthertypeValue");
  use_action_str_set.insert("StadConfigSubtypeEnable");
  use_action_str_set.insert("StadConfigSubtypeValue");
  use_action_str_set.insert("StadConfigVlanId");
  use_action_str_set.insert("StadConfigVlanPCP");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  for (auto if_stad_port_entry : interface_stad_port_entries) {
    for (auto stad_port_entry : if_stad_port_entry.GetStadPortEntries()) {
      // Get Dot1qStaticForward's Snmp
      QList<ActSnmpSetEntry> snmp_set_entry_list;
      QString oid;
      QString value;
      QString oid_body = QString("%1.%2").arg(stad_port_entry.GetPortId()).arg(stad_port_entry.GetIngressIndex());

      // StadConfigIndexEnable
      oid = QString("%1.%2").arg(action_oid_map["StadConfigIndexEnable"]).arg(oid_body);
      value = QString::number(stad_port_entry.GetIndexEnable());
      snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));  // INTEGER(true(1), false(2))
      if (value == "1") {                                               // add
        // StadConfigEthertypeValue
        oid = QString("%1.%2").arg(action_oid_map["StadConfigEthertypeValue"]).arg(oid_body);
        value = QString::number(stad_port_entry.GetEthertypeValue());
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

        // StadConfigSubtypeEnable
        oid = QString("%1.%2").arg(action_oid_map["StadConfigSubtypeEnable"]).arg(oid_body);
        value = QString::number(stad_port_entry.GetSubtypeEnable());
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

        // StadConfigSubtypeValue
        oid = QString("%1.%2").arg(action_oid_map["StadConfigSubtypeValue"]).arg(oid_body);
        value = QString::number(stad_port_entry.GetSubtypeValue());
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

        // StadConfigVlanId
        oid = QString("%1.%2").arg(action_oid_map["StadConfigVlanId"]).arg(oid_body);
        value = QString::number(stad_port_entry.GetVlanId());
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

        // StadConfigVlanPCP
        oid = QString("%1.%2").arg(action_oid_map["StadConfigVlanPCP"]).arg(oid_body);
        value = QString::number(stad_port_entry.GetVlanPcp());
        snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
      }

      // SNMP Set
      act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << __func__ << "SetSnmp(StadPort) failed. device:" << device.GetIpv4().GetIpAddress();
        return act_status;
      }
    }
  }
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetStadConfig(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                         QSet<ActStadConfigEntry> &stad_config_entries) {
  ACT_STATUS_INIT();

  stad_config_entries.clear();
  QMap<QString, QString> snmp_message_result_map;

  QString action_key = "StadConfigEgressUntag";
  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Send SNMP request
  act_status = SendRequestAndInsertResultMap(device, action_key, action.GetPath(), snmp_message_result_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();

    QString port = map_it.key().mid(action.GetPath().size() + 1);
    stad_config_entries.insert(ActStadConfigEntry(port.toInt(), map_it.value().toInt()));
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetStadConfig(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                         const QSet<ActStadConfigEntry> &stad_config_entries) {
  ACT_STATUS_INIT();

  QString action_key = "StadConfigEgressUntag";
  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Handle each entry
  for (auto entry : stad_config_entries) {
    QList<ActSnmpSetEntry> snmp_set_entry_list;
    QString oid = QString("%1.%2").arg(action.GetPath()).arg(entry.GetPortId());
    QString value = QString::number(entry.GetEgressUntag());
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

    // SNMP Set
    act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__
                  << QString("SetSnmp(%1) failed. Device: %2").arg(action_key).arg(device.GetIpv4().GetIpAddress());
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActSnmpHandler::GetDot1qStaticUnicast(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                 QSet<ActStaticForwardEntry> &static_forward_entries) {
  ACT_STATUS_INIT();
  static_forward_entries.clear();

  QMap<QString, QString> snmp_message_result_map;

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Dot1qStaticUnicastStatus");
  use_action_str_set.insert("Dot1qStaticUnicastAllowedToGoTo");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    // Dot1qStaticUnicastAllowedToGoTo
    // request  mib = 1.3.6.1.2.1.17.7.1.3.1.1.3
    // respones mib = 1.3.6.1.2.1.17.7.1.3.1.1.3.1.16.0.0.0.0.2.0
    // vid 1
    // MAC 16.0.0.0.0.2
    // AllowedToGoTo port 0

    // Get VID & MAC
    QString mib_body = "";
    QString mac = "";
    if (map_it.key().contains(action_oid_map["Dot1qStaticUnicastAllowedToGoTo"])) {
      // "1.3.6.1.2.1.17.7.1.3.1.1.3"
      mib_body = map_it.key().mid(action_oid_map["Dot1qStaticUnicastAllowedToGoTo"].size() + 1);
    } else if (map_it.key().contains(action_oid_map["Dot1qStaticUnicastStatus"])) {
      // "1.3.6.1.2.1.17.7.1.3.1.1.4"
      mib_body = map_it.key().mid(action_oid_map["Dot1qStaticUnicastStatus"].size() + 1);
    } else {
      qCritical() << __func__ << "Reply not defined";
      return std::make_shared<ActStatusInternalError>("SNMP");
    }

    QStringList mib_body_list = mib_body.split('.');  // ["1" ,"3" ,"6" ,...]
    //  VID
    qint32 vid = mib_body_list.at(0).toInt();
    //  MAC
    for (int i = 1; i < 7; i++) {
      int int_10base = mib_body_list.at(i).toInt();
      QString hex_str = int_10base > 15 ? QString::number(int_10base, 16) : "0" + QString::number(int_10base, 16);
      mac += hex_str;  // append hex_str
      if (i != 6) {
        mac += "-";
      }
    }
    mac = mac.toUpper();

    // Find entry by VID & MAC
    ActStaticForwardEntry static_forward_entry(vid, mac);
    auto static_forward_entry_found_it = static_forward_entries.find(static_forward_entry);
    if (static_forward_entry_found_it != static_forward_entries.end()) {  // found
      static_forward_entry = *static_forward_entry_found_it;
      static_forward_entries.erase(static_forward_entry_found_it);
    }

    // Set entry
    if (map_it.key().contains(action_oid_map["Dot1qStaticUnicastAllowedToGoTo"])) {
      // Egress
      static_forward_entry.SetEgressPorts(HexToPorts(map_it.value()));
    } else if (map_it.key().contains(action_oid_map["Dot1qStaticUnicastStatus"])) {
      // Status
      static_forward_entry.SetDot1qStatus(map_it.value().toInt());
    }
    static_forward_entries.insert(static_forward_entry);
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetDot1qStaticUnicast(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                 const QSet<ActStaticForwardEntry> &static_forward_entries) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Dot1qStaticUnicastStatus");
  use_action_str_set.insert("Dot1qStaticUnicastAllowedToGoTo");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  for (auto static_forward_entry : static_forward_entries) {
    QList<ActSnmpSetEntry> snmp_set_entry_list;
    // Get Dot1qStaticForward's SnmpSetEntry List

    QString mac_dec = MacHexToDec(static_forward_entry.GetMAC());
    QString ingress_port = "0";
    QString oid_body = QString("%1.%2.%3").arg(static_forward_entry.GetVlanId()).arg(mac_dec).arg(ingress_port);

    // Dot1qStaticUnicastStatus
    QString oid = QString("%1.%2").arg(action_oid_map["Dot1qStaticUnicastStatus"]).arg(oid_body);
    QString value = QString::number(static_forward_entry.GetDot1qStatus());
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

    if (static_forward_entry.GetDot1qStatus() != 2) {  // invalid(2) -> remove(skip set egress ports)
      // Dot1qStaticUnicastAllowedToGoTo
      oid = QString("%1.%2").arg(action_oid_map["Dot1qStaticUnicastAllowedToGoTo"]).arg(oid_body);
      value = BinToHex(PortsToBin(static_forward_entry.GetEgressPorts()));
      snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'x'));
    }

    // Send SNMP Set
    act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetSnmp(Dot1qStaticUnicast) failed. device:" << device.GetIpv4().GetIpAddress();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetDot1qStaticMulticast(const ActDevice &device,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   QSet<ActStaticForwardEntry> &static_forward_entries) {
  ACT_STATUS_INIT();
  static_forward_entries.clear();

  QMap<QString, QString> snmp_message_result_map;

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Dot1qStaticMulticastStaticEgressPorts");
  use_action_str_set.insert("Dot1qStaticMulticastStatus");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // Generate result entry
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    // Dot1qStaticMulticastStaticEgressPorts
    // request  mib = 1.3.6.1.2.1.17.7.1.3.2.1.3
    // respones mib = 1.3.6.1.2.1.17.7.1.3.2.1.3.1.16.0.0.0.0.2.0
    // vid 1
    // MAC 16.0.0.0.0.2
    // AllowedToGoTo port 0

    // Get VID & MAC
    QString mib_body = "";
    QString mac = "";
    if (map_it.key().contains(action_oid_map["Dot1qStaticMulticastStaticEgressPorts"])) {
      // "1.3.6.1.2.1.17.7.1.3.2.1.3"
      mib_body = map_it.key().mid(action_oid_map["Dot1qStaticMulticastStaticEgressPorts"].size() + 1);
    } else if (map_it.key().contains(action_oid_map["Dot1qStaticMulticastStatus"])) {
      // "1.3.6.1.2.1.17.7.1.3.2.1.5"
      mib_body = map_it.key().mid(action_oid_map["Dot1qStaticMulticastStatus"].size() + 1);
    } else {
      qCritical() << __func__ << "Reply not defined";
      return std::make_shared<ActStatusInternalError>("SNMP");
    }

    QStringList mib_body_list = mib_body.split('.');  // ["1" ,"3" ,"6" ,...]
    //  VID
    qint32 vid = mib_body_list.at(0).toInt();
    //  MAC
    for (int i = 1; i < 7; i++) {
      int int_10base = mib_body_list.at(i).toInt();
      QString hex_str = int_10base > 15 ? QString::number(int_10base, 16) : "0" + QString::number(int_10base, 16);
      mac += hex_str;  // append hex_str
      if (i != 6) {
        mac += "-";
      }
    }
    mac = mac.toUpper();

    // Find entry by VID & MAC
    ActStaticForwardEntry static_forward_entry(vid, mac);
    auto static_forward_entry_found_it = static_forward_entries.find(static_forward_entry);
    if (static_forward_entry_found_it != static_forward_entries.end()) {  // found
      static_forward_entry = *static_forward_entry_found_it;
      static_forward_entries.erase(static_forward_entry_found_it);
    }

    // Set entry
    if (map_it.key().contains(action_oid_map["Dot1qStaticMulticastStaticEgressPorts"])) {
      // Egress
      static_forward_entry.SetEgressPorts(HexToPorts(map_it.value()));
    } else if (map_it.key().contains(action_oid_map["Dot1qStaticMulticastStatus"])) {
      // Status
      static_forward_entry.SetDot1qStatus(map_it.value().toInt());
    }
    static_forward_entries.insert(static_forward_entry);
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetDot1qStaticMulticast(const ActDevice &device,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const QSet<ActStaticForwardEntry> &static_forward_entries) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("Dot1qStaticMulticastStatus");
  use_action_str_set.insert("Dot1qStaticMulticastStaticEgressPorts");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate result entry
  for (auto static_forward_entry : static_forward_entries) {
    QList<ActSnmpSetEntry> snmp_set_entry_list;
    // Get Dot1qStaticForward's SnmpSetEntry List

    QString mac_dec = MacHexToDec(static_forward_entry.GetMAC());
    QString ingress_port = "0";
    QString oid_body = QString("%1.%2.%3").arg(static_forward_entry.GetVlanId()).arg(mac_dec).arg(ingress_port);

    // Dot1qStaticMulticastStatus
    QString oid = QString("%1.%2").arg(action_oid_map["Dot1qStaticMulticastStatus"]).arg(oid_body);
    QString value = QString::number(static_forward_entry.GetDot1qStatus());
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

    if (static_forward_entry.GetDot1qStatus() != 2) {  // invalid(2) -> remove(skip set egress ports)
      // Dot1qStaticMulticastStaticEgressPorts
      oid = QString("%1.%2").arg(action_oid_map["Dot1qStaticMulticastStaticEgressPorts"]).arg(oid_body);
      value = BinToHex(PortsToBin(static_forward_entry.GetEgressPorts()));
      snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'x'));
    }

    // Send SNMP Set
    act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << __func__ << "SetSnmp(Dot1qStaticUnicast) failed. device:" << device.GetIpv4().GetIpAddress();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetSpanningTreeMethod(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                 const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("RstpConfigEnable");
  use_action_str_set.insert("Ieee8021SpanningTreeVersion");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Active
  QString oid = QString("%1.0").arg(action_oid_map["RstpConfigEnable"]);
  QString value = rstp_table.GetActive() ? "1" : "2";  // value: true(1), false(2)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Version
  oid = QString("%1.1").arg(action_oid_map["Ieee8021SpanningTreeVersion"]);
  value = QString::number(2);  // value: stp(0), rstp(2), mstp(3)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(SpanningTree Active & Version) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetTWSSpanningTreeMethod(const ActDevice &device,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("EnableSpanningTree");
  use_action_str_set.insert("ProtocolOfRedundancySetup");

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Redundancy Protocol
  // eCos switch not support stp only rstp
  QString oid = QString("%1.0").arg(action_oid_map["ProtocolOfRedundancySetup"]);
  QString value = QString::number(1);  // value: spanningTree(1), turboRing(2), turboRingV2(3), turboChain(4), mstp(5)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // // Per Port active
  qint64 size = device.GetInterfaces().size();
  for (int intf_id = 1; intf_id <= size; intf_id++) {
    QString oid = QString("%1.%2").arg(action_oid_map["EnableSpanningTree"]).arg(intf_id);
    QString value = rstp_table.GetActive() ? "1" : "0";  // value: disable(0), enable(1)
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
  }

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(SpanningTree Active & Version) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetSpanningTreeMethod(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
                                                 ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  QMap<QString, QString> snmp_message_result_map;

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("RstpConfigEnable");
  use_action_str_set.insert("Ieee8021SpanningTreeVersion");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // TODO: To assign value to rstp_table
  QSet<QString> action_check_reply_set;  // for check reply
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    QString mib_body = "";
    if (map_it.key().contains(action_oid_map["RstpConfigEnable"])) {
      auto rstp_enable = map_it.value();
      action_check_reply_set.insert("RstpConfigEnable");
    } else if (map_it.key().contains(action_oid_map["Ieee8021SpanningTreeVersion"])) {
      auto rstp_version = map_it.value();
      action_check_reply_set.insert("Ieee8021SpanningTreeVersion");
    } else {
      qCritical() << __func__ << "Reply not defined";
      return std::make_shared<ActStatusInternalError>("SNMP");
    }
  }

  for (auto use_action : use_action_str_set) {
    if (!action_check_reply_set.contains(use_action)) {
      return NotFoundReturnMessageErrorHandler(__func__, use_action, action_oid_map[use_action], device);
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::GetTWSSpanningTreeMethod(const ActDevice &device,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  QMap<QString, QString> snmp_message_result_map;

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert("EnableSpanningTree");
  use_action_str_set.insert("ProtocolOfRedundancySetup");
  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Send SNMP request
  for (auto action_key : use_action_str_set) {
    act_status = SendRequestAndInsertResultMap(device, action_key, action_oid_map[action_key], snmp_message_result_map);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  // TODO: To assign value to rstp_table
  QSet<QString> action_check_reply_set;  // for check reply
  QMapIterator<QString, QString> map_it(snmp_message_result_map);
  while (map_it.hasNext()) {
    map_it.next();
    QString mib_body = "";
    if (map_it.key().contains(action_oid_map["EnableSpanningTree"])) {
      auto rstp_enable = map_it.value();
      action_check_reply_set.insert("EnableSpanningTree");
    } else if (map_it.key().contains(action_oid_map["ProtocolOfRedundancySetup"])) {
      auto rstp_version = map_it.value();
      action_check_reply_set.insert("ProtocolOfRedundancySetup");
    } else {
      qCritical() << __func__ << "Reply not defined";
      return std::make_shared<ActStatusInternalError>("SNMP");
    }
  }

  for (auto use_action : use_action_str_set) {
    if (!action_check_reply_set.contains(use_action)) {
      return NotFoundReturnMessageErrorHandler(__func__, use_action, action_oid_map[use_action], device);
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetIeee8021SpanningTreeBridgeHelloTime(const ActDevice &device, const QString &action_key,
                                                                  const ActFeatureMethodProtocol &protocol_elem,
                                                                  const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert(action_key);

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Hello Time
  // Ieee8021SpanningTreeBridgeHelloTime
  QString oid = QString("%1.1").arg(action_oid_map[action_key]);
  QString value = QString::number(rstp_table.GetHelloTime() * 100);  // value: 100(1秒), 200(2秒)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(Ieee8021SpanningTree HelloTime) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetSpanningTreeHelloTime(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActRstpTable &rstp_table) {
  // ECos TWS switch
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert(action_key);

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Hello Time
  QString oid = QString("%1.0").arg(action_oid_map[action_key]);
  QString value = QString::number(rstp_table.GetHelloTime());
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(SpanningTree HelloTime) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetIeee8021SpanningTreePriority(const ActDevice &device, const QString &action_key,
                                                           const ActFeatureMethodProtocol &protocol_elem,
                                                           const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert(action_key);

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Priority
  QString oid = QString("%1.1").arg(action_oid_map[action_key]);
  QString value = QString::number(rstp_table.GetPriority());  // Value: 4096(要4096倍數)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(SpanningTree Priority) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetSpanningTreePriority(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem,
                                                   const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Ecos switch

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert(action_key);

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Priority
  QString oid = QString("%1.0").arg(action_oid_map[action_key]);
  QString value = QString::number(rstp_table.GetPriority());  // Value: 4096(要4096倍數)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(SpanningTree Priority) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetSpanningTreeRootGuard(const ActDevice &device, const QString &action_key,
                                                    const ActFeatureMethodProtocol &protocol_elem,
                                                    const ActRstpTable &rstp_table) {
  ACT_STATUS_INIT();

  // Create use action's oid map
  QSet<QString> use_action_str_set;
  QMap<QString, QString> action_oid_map;
  use_action_str_set.insert(action_key);

  act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Root Guard
  for (auto entry : rstp_table.GetRstpPortEntries()) {
    QString oid = QString("%1.%2").arg(action_oid_map[action_key]).arg(entry.GetPortId());
    QString value = entry.GetRootGuard() ? "1" : "2";  // true(1) , false(2)
    snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
  }
  // qint64 size = device.GetInterfaces().size();
  // for (int intf_id = 1; intf_id <= size; intf_id++) {
  //   QString oid = QString("%1.%2").arg(action_oid_map[action_key]).arg(intf_id);
  //   QString value = rstp_table.GetRootGuards().contains(intf_id) ? "1" : "2";  // true(1) , false(2)
  //   snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
  // }

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(SpanningTree RootGuard) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetEnableWarmStart(const ActDevice &device, const QString &action_key,
                                              const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Enable yes(1)
  const QString oid = QString("%1.0").arg(action.GetPath());
  const QString value = "1";  // yes(1)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(EnableWarmStart) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SetEnableFactoryDefault(const ActDevice &device, const QString &action_key,
                                                   const ActFeatureMethodProtocol &protocol_elem) {
  ACT_STATUS_INIT();

  // Get Action element
  if (!protocol_elem.GetActions().contains(action_key)) {
    qCritical() << __func__ << QString("The %1 action not found").arg(action_key);
    return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(action_key));
  }
  auto action = protocol_elem.GetActions()[action_key];

  // Handle each entry
  QList<ActSnmpSetEntry> snmp_set_entry_list;

  // Enable activate(1)
  const QString oid = QString("%1.0").arg(action.GetPath());
  const QString value = "1";  // activate(1)
  snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

  // Send SNMP Set
  act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Set(EnableFactoryDefault) failed. device:" << device.GetIpv4().GetIpAddress();
    return act_status;
  }

  return act_status;
}

// ACT_STATUS ActSnmpHandler::SetSpanningTree(const ActDevice &device, const ActFeatureMethodProtocol &protocol_elem,
//                                            const ActRstpTable &rstp_table) {
//   ACT_STATUS_INIT();

//   bool support_root_guard = false;
//   if (protocol_elem.GetActions().contains("RstpConfigPortRootGuard")) {
//     support_root_guard = true;
//   }

//   // Create use item's oid map
//   QSet<QString> use_action_str_set;
//   QMap<QString, QString> action_oid_map;
//   use_action_str_set.insert("RstpConfigEnable");
//   if (support_root_guard) {
//     use_action_str_set.insert("RstpConfigPortRootGuard");
//   }
//   use_action_str_set.insert("Ieee8021SpanningTreeVersion");
//   use_action_str_set.insert("Ieee8021SpanningTreeBridgeHelloTime");
//   use_action_str_set.insert("Ieee8021SpanningTreePriority");
//   act_status = CreateActionOidMap(protocol_elem, use_action_str_set, action_oid_map);
//   if (!IsActStatusSuccess(act_status)) {
//     return act_status;
//   }

//   // Handle each entry
//   QList<ActSnmpSetEntry> snmp_set_entry_list;

//   // Active
//   QString oid = QString("%1.0").arg(action_oid_map["RstpConfigEnable"]);
//   QString value = QString::number(rstp_table.GetActive() ? 1 : 2);  // value: true(1), false(2)
//   snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

//   // Version
//   oid = QString("%1.1").arg(action_oid_map["Ieee8021SpanningTreeVersion"]);
//   value = QString::number(2);  // value: stp(0), rstp(2), mstp(3)
//   snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

//   // Hello Time
//   oid = QString("%1.1").arg(action_oid_map["Ieee8021SpanningTreeBridgeHelloTime"]);
//   value = QString::number(rstp_table.GetHelloTime() * 100);  // value: 100(1秒), 200(2秒)
//   snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

//   // Priority
//   oid = QString("%1.1").arg(action_oid_map["Ieee8021SpanningTreePriority"]);
//   value = QString::number(rstp_table.GetPriority());  // Value: 4096(要4096倍數)
//   snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));

//   // Root Guard
//   if (support_root_guard) {
//     if (device.GetDeviceType() == ActDeviceTypeEnum::kSwitch) {
//       qint64 size = device.GetInterfaces().size();
//       for (int intf_id = 1; intf_id <= size; intf_id++) {
//         QString oid = QString("%1.%2").arg(action_oid_map["RstpConfigPortRootGuard"]).arg(intf_id);
//         QString value = rstp_table.GetRootGuards().contains(intf_id) ? QString::number(true) :
//         QString::number(false); snmp_set_entry_list.push_back(ActSnmpSetEntry(oid, value, 'i'));
//       }
//     }
//   }

//   // Send SNMP Set
//   act_status = ActSnmpset::SetSnmp(snmp_set_entry_list, device);
//   if (!IsActStatusSuccess(act_status)) {
//     qCritical() << __func__ << "Set(SpanningTree) failed. device:" << device.GetIpv4().GetIpAddress();
//     return act_status;
//   }

//   return ACT_STATUS_SUCCESS;
// }

//////// Refactor end

// ACT_STATUS ActSnmpHandler::Get8021AsPortDSNeighborPropDelayFromMsLs(const ActDevice &device,
//                                                                     QSet<ActPortOidValue> &port_oid_value_set) {
//   ACT_STATUS_INIT();
//   port_oid_value_set.clear();

//   // PropagationDelay item (1 ~ 204000 ns)
//   // HS : MS : LS ( 65535 ~ 2^96)
//   //  LS: 2^16 ~ 2^32
//   //  MS: 2^32 ~ 2^64
//   //  HS: 2^64 ~ 2^96
//   //  so 204000 only get MS + LS

//   // ActSnmpResult<ActSnmpMessageMap> snmp_result_hs;
//   // act_status = ActSnmpwalk::GetSnmpSubTree(kIeee8021AsPortDSNeighborPropDelayHsOid,
//   //                                                           device.GetIpv4().GetIpAddress(), snmp_result_hs);
//   ActSnmpResult<ActSnmpMessageMap> snmp_result_ms;
//   act_status = ActSnmpwalk::GetSnmpSubTree(kIeee8021AsPortDSNeighborPropDelayMsOid, device, snmp_result_ms);
//   ActSnmpResult<ActSnmpMessageMap> snmp_result_ls;
//   act_status = ActSnmpwalk::GetSnmpSubTree(kIeee8021AsPortDSNeighborPropDelayLsOid, device, snmp_result_ls);

//   if (!IsActStatusSuccess(act_status)) {
//     qCritical() << __func__
//                 << "GetSnmpSubTree(8021AsPropagationDelayFromMsLs) failed. device:" <<
//                 device.GetIpv4().GetIpAddress();
//     return act_status;
//   }

//   //  request mib = 1.3.111.2.802.1.1.20.1.5.1.9
//   //  request mib = 1.3.111.2.802.1.1.20.1.5.1.10
//   //  request mib = 1.3.111.2.802.1.1.20.1.5.1.11
//   //  respones mib= 1.3.111.2.802.1.1.20.1.5.1.11.1.1 (port1)
//   //  respones mib= 1.3.111.2.802.1.1.20.1.5.1.11.2.2 (port2)

//   QMapIterator<QString, QString> map_ms_it(snmp_result_ms.GetSnmpMessage());
//   while (map_ms_it.hasNext()) {
//     map_ms_it.next();
//     QString mib_body = map_ms_it.key().mid(kIeee8021AsPortDSNeighborPropDelayMsOid.size() + 1);  // "1.1"
//     QStringList mib_body_list = mib_body.split('.');                                             // ["1", "1"]
//     QString port = mib_body_list.at(0);

//     QString map_key_ls = QString("%1.%2").arg(kIeee8021AsPortDSNeighborPropDelayLsOid).arg(mib_body);

//     QString map_value_ms = map_ms_it.value();
//     QString map_value_ls = snmp_result_ls.GetSnmpMessage().value(map_key_ls);

//     // str_10base to str_16base
//     // QString str_16base_ms = "3";
//     // QString str_16base_ls = "1CE00000";
//     QString str_16base_ms = QString::number(map_value_ms.toUInt(), 16);
//     QString str_16base_ls = QString::number(map_value_ls.toUInt(), 16);
//     QString str_16base_mls = str_16base_ms + str_16base_ls;
//     quint32 delay = str_16base_mls.toULongLong(nullptr, 16) / 65536;
//     // std::stringstream sstream;
//     // sstream << std::hex << strtoull(map_value_ls.toStdString().c_str(), nullptr, 10);
//     // uint64_t delay_i = strtoull(str_16base_mls.toStdString().c_str(), nullptr, 16) / 65536;
//     // 3 1CE0 0000

//     // qDebug() << QString("Port %1: ms:%3, ls:%4 (%5) --- (%6)")
//     //                 .arg(port)
//     //                 .arg(map_value_ms)
//     //                 .arg(map_value_ls)
//     //                 .arg(str_16base_mls)
//     //                 .arg(delay);

//     port_oid_value_set.insert(ActPortOidValue(port, QString::number(delay)));
//   }
//   return act_status;
// }

ACT_STATUS ActSnmpHandler::SendRequestAndInsertResultMap(const ActDevice &device, const QString &action_str,
                                                         const QString &action_oid,
                                                         QMap<QString, QString> &snmp_message_result_map) {
  ACT_STATUS_INIT();

  ActSnmpResult<ActSnmpMessageMap> get_snmp_result(device.GetIpv4().GetIpAddress());

  // Send SNMP request
  get_snmp_result = ActSnmpResult<ActSnmpMessageMap>(device.GetIpv4().GetIpAddress());
  act_status = ActSnmpwalk::GetSnmpSubTree(action_oid, device, get_snmp_result);
  if (!IsActStatusSuccess(act_status)) {
    return GetSnmpSubTreeErrorHandler(__func__, act_status, action_str, action_oid, device);
  }
  // merge SnmpMessage map
  foreach (auto key, get_snmp_result.GetSnmpMessage().keys()) {
    snmp_message_result_map.insert(key, get_snmp_result.GetSnmpMessage()[key]);
  }
  // qDebug() << __func__ << QString("get_snmp_result(%1):").arg(action_str)
  //          << get_snmp_result.ToString().toStdString().c_str();

  // qDebug() << __func__
  //          << QString("Device(%1) get_snmp_result(next req)(%2)size: %3")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(action_str)
  //                 .arg(snmp_message_result_map.size());

  return act_status;
}

ACT_STATUS ActSnmpHandler::SendRequestAndInsertResultMapByBulk(const ActDevice &device, const QString &action_str,
                                                               const QString &action_oid,
                                                               QMap<QString, QString> &snmp_message_result_map) {
  ACT_STATUS_INIT();

  ActSnmpResult<ActSnmpMessageMap> get_snmp_result(device.GetIpv4().GetIpAddress());

  // Send SNMP request
  get_snmp_result = ActSnmpResult<ActSnmpMessageMap>(device.GetIpv4().GetIpAddress());
  act_status = ActSnmpwalk::GetSnmpSubTreeByBulk(action_oid, device, get_snmp_result);
  if (!IsActStatusSuccess(act_status)) {
    return GetSnmpSubTreeErrorHandler(__func__, act_status, action_str, action_oid, device);
  }
  // merge SnmpMessage map
  foreach (auto key, get_snmp_result.GetSnmpMessage().keys()) {
    snmp_message_result_map.insert(key, get_snmp_result.GetSnmpMessage()[key]);
  }
  // qDebug() << __func__ << QString("get_snmp_result(Bulk request)(%1):").arg(action_str)
  //          << get_snmp_result.ToString().toStdString().c_str();

  // qDebug() << __func__
  //          << QString("Device(%1) get_snmp_result(bulk req)(%2) size: %3")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(action_str)
  //                 .arg(snmp_message_result_map.size());

  return act_status;
}

ACT_STATUS ActSnmpHandler::StadInedexEnableSnmpMessageMapToSnmpRequestList(
    const QMap<QString, QString> &item_oid_map, const QMap<QString, QString> &snmp_message_result_map,
    QMap<QString, QString> &enabled_index_snmp_message_map, QList<QString> &request_oid_list) {
  ACT_STATUS_INIT();

  try {
    // Generate result entry
    QMapIterator<QString, QString> map_it(snmp_message_result_map);
    while (map_it.hasNext()) {
      map_it.next();
      if (map_it.value() == "2") {  // true(1), false(2)
        // Only get the index that InedexEnable is enabled.
        continue;
      }
      enabled_index_snmp_message_map.insert(map_it.key(), map_it.value());

      // request  mib = 1.3.6.1.4.1.8691.603.2.10.1.2.1.3
      // respones mib = 1.3.6.1.4.1.8691.603.2.10.1.2.1.3.1.1
      QString port_index =
          map_it.key().mid(item_oid_map["StadConfigIndexEnable"].size() + 1);  // "1.1" => ["port_id", "ingress_index"]

      // EthertypeValue
      request_oid_list.push_back(QString("%1.%2").arg(item_oid_map["StadConfigEthertypeValue"]).arg(port_index));
      // SubtypeEnable
      request_oid_list.push_back(QString("%1.%2").arg(item_oid_map["StadConfigSubtypeEnable"]).arg(port_index));
      // SubtypeValue
      request_oid_list.push_back(QString("%1.%2").arg(item_oid_map["StadConfigSubtypeValue"]).arg(port_index));
      // VlanId
      request_oid_list.push_back(QString("%1.%2").arg(item_oid_map["StadConfigVlanId"]).arg(port_index));
      // VlanPCP
      request_oid_list.push_back(QString("%1.%2").arg(item_oid_map["StadConfigVlanPCP"]).arg(port_index));
    }

  } catch (std::exception &e) {
    qCritical() << __func__ << "StadInedexEnableSnmpMessageMapToSnmpRequestList() failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::SnmpMessageMapToInterfaceStadPort(
    const QMap<QString, QString> &item_oid_map, const QMap<QString, QString> &snmp_message_map,
    QSet<ActInterfaceStadPortEntry> &interface_stad_port_entries) {
  ACT_STATUS_INIT();
  interface_stad_port_entries.clear();
  QMapIterator<QString, QString> map_it(snmp_message_map);

  while (map_it.hasNext()) {
    map_it.next();
    // StadConfigIndexEnable
    // request  mib = 1.3.6.1.4.1.8691.603.2.10.1.2.1.3
    // respones mib = 1.3.6.1.4.1.8691.603.2.10.1.2.1.3.1.1
    //  ["1" , "1"] => ["port_id", "ingress_index"]

    // Get port_id & ingress_index
    QString mib_body = "";
    if (map_it.key().contains(item_oid_map["StadConfigIndexEnable"])) {
      // 1.3.6.1.4.1.8691.603.2.10.1.2.1.3
      mib_body = map_it.key().mid(item_oid_map["StadConfigIndexEnable"].size() + 1);
    } else if (map_it.key().contains(item_oid_map["StadConfigEthertypeValue"])) {
      // 1.3.6.1.4.1.8691.603.2.10.1.2.1.4
      mib_body = map_it.key().mid(item_oid_map["StadConfigEthertypeValue"].size() + 1);
    } else if (map_it.key().contains(item_oid_map["StadConfigSubtypeEnable"])) {
      // 1.3.6.1.4.1.8691.603.2.10.1.2.1.5
      mib_body = map_it.key().mid(item_oid_map["StadConfigSubtypeEnable"].size() + 1);
    } else if (map_it.key().contains(item_oid_map["StadConfigSubtypeValue"])) {
      // 1.3.6.1.4.1.8691.603.2.10.1.2.1.6
      mib_body = map_it.key().mid(item_oid_map["StadConfigSubtypeValue"].size() + 1);
    } else if (map_it.key().contains(item_oid_map["StadConfigVlanId"])) {
      // 1.3.6.1.4.1.8691.603.2.10.1.2.1.7
      mib_body = map_it.key().mid(item_oid_map["StadConfigVlanId"].size() + 1);
    } else if (map_it.key().contains(item_oid_map["StadConfigVlanPCP"])) {
      // 1.3.6.1.4.1.8691.603.2.10.1.2.1.8
      mib_body = map_it.key().mid(item_oid_map["StadConfigVlanPCP"].size() + 1);
    } else {
      qCritical() << __func__ << "Reply not defined";
      return std::make_shared<ActStatusInternalError>("SNMP");
    }

    QStringList mib_body_list = mib_body.split('.');  // ["1" , "1"] => [""port_id", "ingress_index"]
    qint64 port_id = mib_body_list.at(0).toInt();
    qint32 ingress_index = mib_body_list.at(1).toInt();

    QSet<ActStadPortEntry> stad_port_entry_set;  // ActInterfaceStadPortEntry's stad_port_entryies
    ActStadPortEntry stad_port_entry(port_id, ingress_index);
    // Find stad_port_entry
    // First to find interface_stad_port_entry
    ActInterfaceStadPortEntry if_stad_port_entry(port_id);
    auto if_stad_port_entry_iter = interface_stad_port_entries.find(if_stad_port_entry);
    if (if_stad_port_entry_iter != interface_stad_port_entries.end()) {  // found
      stad_port_entry_set = if_stad_port_entry_iter->GetStadPortEntries();
      interface_stad_port_entries.erase(if_stad_port_entry_iter);
      // Find the stad_port_entry
      auto stad_port_entry_iter = stad_port_entry_set.find(stad_port_entry);
      if (stad_port_entry_iter != stad_port_entry_set.end()) {  // found
        stad_port_entry = *stad_port_entry_iter;
        stad_port_entry_set.erase(stad_port_entry_iter);
      }
    }

    // Set entry
    if (map_it.key().contains(item_oid_map["StadConfigIndexEnable"])) {
      // IndexEnable
      stad_port_entry.SetIndexEnable(map_it.value().toInt());
    } else if (map_it.key().contains(item_oid_map["StadConfigEthertypeValue"])) {
      // EthertypeValue
      stad_port_entry.SetEthertypeValue(map_it.value().toInt());
    } else if (map_it.key().contains(item_oid_map["StadConfigSubtypeEnable"])) {
      // SubtypeEnable
      stad_port_entry.SetSubtypeEnable(map_it.value().toInt());
    } else if (map_it.key().contains(item_oid_map["StadConfigSubtypeValue"])) {
      // SubtypeValue
      stad_port_entry.SetSubtypeValue(map_it.value().toInt());
    } else if (map_it.key().contains(item_oid_map["StadConfigVlanId"])) {
      // VlanId
      stad_port_entry.SetVlanId(map_it.value().toInt());
    } else if (map_it.key().contains(item_oid_map["StadConfigVlanPCP"])) {
      // VlanPcp
      stad_port_entry.SetVlanPcp(map_it.value().toInt());
    }

    // Assign to ActInterfaceStadPortEntry
    stad_port_entry_set.insert(stad_port_entry);
    if_stad_port_entry.SetStadPortEntries(stad_port_entry_set);
    interface_stad_port_entries.insert(if_stad_port_entry);
  }
  return act_status;
}

ACT_STATUS ActSnmpHandler::CreateActionOidMap(const ActFeatureMethodProtocol &protocol_elem,
                                              const QSet<QString> &use_action_key_str_set,
                                              QMap<QString, QString> &result_action_oid_map) {
  ACT_STATUS_INIT();

  for (auto use_action_key_str : use_action_key_str_set) {
    if (!protocol_elem.GetActions().contains(use_action_key_str)) {
      qCritical() << __func__ << QString("The %1 action not found").arg(use_action_key_str);
      return std::make_shared<ActStatusNotFound>(QString("Action(%1)").arg(use_action_key_str));
    }

    result_action_oid_map.insert(use_action_key_str, protocol_elem.GetActions()[use_action_key_str].GetPath());
  }

  return act_status;
}

ACT_STATUS ActSnmpHandler::LldpRemOidToPort(const QString &oid, const QString &response_oid, qint64 &port) {
  ACT_STATUS_INIT();

  // request  mib = 1.0.8802.1.1.2.1.4.1.1.5
  // respones mib = 1.0.8802.1.1.2.1.4.1.1.5.0.5.1
  // port 5
  QString mib_body = response_oid.mid(oid.size() + 1);  // "0.5.1"
  QStringList mib_body_list = mib_body.split('.');      // ["0", "5", "1"]
  port = mib_body_list.at(1).toInt();

  return act_status;
}

QString ActSnmpHandler::MacDecToHex(const QString &mac_dec) {
  // 16.00.00.06.232.144 -> 10-00-00-06-E8-90
  QStringList mac_dec_list = mac_dec.split('.');  // ["16", "00", "00", "06", "232", "144"]
  QString mac = "";
  // for (auto mac_dec : mac_dec_list) {
  for (int i = 0; i < 6; i++) {
    int int_10base = mac_dec_list.at(i).toInt();
    QString hex_str = int_10base > 15 ? QString::number(int_10base, 16) : "0" + QString::number(int_10base, 16);

    mac += hex_str;  // append hex_str
    if (i != 5) {
      mac += "-";
    }
  }
  mac = mac.toUpper();
  return mac;
}

QString ActSnmpHandler::MacHexToDec(const QString &mac_hex) {
  // 10-00-00-06-24-11 -> 16.00.00.06.36.17
  QString result;
  for (int i = 0; i < mac_hex.length(); i += 3) {
    bool ok;
    QString mac_hex_str = QString(mac_hex.at(i)) + QString(mac_hex.at(i + 1));  // "10"
    int int_16base = mac_hex_str.toInt(&ok, 16);                                // "10" -> 10

    QString dec_str = QString::number(int_16base, 10);  // 10 -> "16"
    // if (dec_str.length() < 2) {                      // "6" -> "06"
    //   dec_str = "0" + dec_str;
    // }

    result.append(dec_str).append(".");
  }
  // remove last dot.
  result = result.mid(0, result.length() - 1);
  return result;
}

QString ActSnmpHandler::PortsToBin(const QSet<qint64> &ports) {
  int max = 128;
  QString bin;

  for (int i = 1; i <= max; i++) {
    if (ports.find(i) != ports.end()) {
      bin.push_back('1');
    } else {
      bin.push_back('0');
    }
  }

  return bin;
}

QString ActSnmpHandler::BinToHex(const QString &bin) {
  QString paddedBinaryString = bin.rightJustified((bin.length() / 4) * 4, '0');
  QByteArray byteArray;
  QString hexString = "0x";

  for (int i = 0; i < paddedBinaryString.length(); i += 4) {
    QString nibbleString = paddedBinaryString.mid(i, 4);

    int int_2base = nibbleString.toInt(nullptr, 2);
    QString hex_qstr = QString::number(int_2base, 16).toUpper();
    hexString = hexString + hex_qstr;
  }
  return hexString;
}

QSet<qint64> ActSnmpHandler::BinToPorts(const QString &bin) {
  QSet<qint64> ports;
  for (int i = 0; i < bin.length(); i++) {
    if (bin[i] == '1') {
      ports.insert(i + 1);
    }
  }
  return ports;
}

QSet<qint64> ActSnmpHandler::HexToPorts(const QString &hex) {
  // hex string -> int 16base -> bin string -> set<qint64> port

  // Hex string to Binary String
  QByteArray byteArray = QByteArray::fromHex(hex.toLatin1());
  QString binaryString;

  for (char byte : byteArray) {
    QString byteString = QString::number(static_cast<unsigned char>(byte), 2).rightJustified(8, '0');
    binaryString.append(byteString);
  }
  return BinToPorts(binaryString);
}

QSet<QString> ActSnmpHandler::PortsIdToPortsIndex(const QSet<QString> &ports_id) {
  QSet<QString> ports_index;
  for (auto port_id : ports_id) {
    ports_index.insert(QString::number(port_id.toInt() + 1));
  }
  return ports_index;
}

QSet<QString> ActSnmpHandler::PortsIndexToPortsId(const QSet<QString> &ports_index) {
  QSet<QString> ports_id;
  for (auto port_index : ports_index) {
    ports_id.insert(QString::number(port_index.toInt() - 1));
  }
  return ports_id;
}
