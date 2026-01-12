/* Copyright (C) MOXA Inc. All rights reserved.
 This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
 See the file MOXA-SOFTWARE-NOTICE for details.
 */

#ifndef ACT_SNMP_RESULT_HPP
#define ACT_SNMP_RESULT_HPP
#include "act_json.hpp"

/**
 * @brief The SNMP result template derives from a variety of result types
 *
 * @tparam T The derives class
 */
template <typename T>
class ActSnmpResult : public T {
 public:
  ActSnmpResult() : T() {};
  ActSnmpResult(QString device_ip) : T(device_ip) {};
};

/**
 * @brief The SNMP result base class
 *
 */
class ActSnmpResultBase : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_ip, DeviceIp);  ///< The device IP of SNMP server (TSNSwich)
  // ACT_JSON_FIELD(QString, result_body, ResultBody);  ///< The qstring result body

 public:
  /**
   * @brief Construct a new Snmp Result Base object
   *
   */
  ActSnmpResultBase() {};
  /**
   * @brief Construct a new Snmp Result Base object
   *
   * @param device_ip
   */
  ActSnmpResultBase(QString device_ip) : device_ip_(device_ip) {};
};

class ActSnmpMessageMap : public ActSnmpResultBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, QString, QString, snmp_message, SnmpMessage);  ///< The SnmpMessage map<oid, value>
 public:
  ActSnmpMessageMap() : ActSnmpResultBase() {};
  ActSnmpMessageMap(QString device_ip) : ActSnmpResultBase(device_ip) {};
};

class ActSnmpMessageString : public ActSnmpResultBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, snmp_message, SnmpMessage);  ///< The SnmpMessage map<oid, value>
 public:
  ActSnmpMessageString() : ActSnmpResultBase() {};
  ActSnmpMessageString(QString device_ip) : ActSnmpResultBase(device_ip) {};
};

class ActPortOidValue : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, port_oid, PortOid);  ///< The PortOid item

  ACT_JSON_FIELD(QString, value, Value);  ///< The Value item

 public:
  /**
   * @brief Construct a new Port Oid Value object
   *
   */
  ActPortOidValue() : port_oid_("0") {};

  /**
   * @brief Construct a new Act Port Oid Value object
   *
   * @param port_oid
   */
  ActPortOidValue(QString port_oid) : port_oid_(port_oid) {};

  /**
   * @brief Construct a new Port Oid Value object
   *
   */
  ActPortOidValue(QString port_oid, QString value) : port_oid_(port_oid), value_(value) {};

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActPortOidValue &x) {
    return qHash(x.port_oid_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActPortOidValue &x, const ActPortOidValue &y) { return x.port_oid_ == y.port_oid_; }
};

#endif /* ACT_SNMP_RESULT_HPP */