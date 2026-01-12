/* Copyright (C) MOXA Inc. All rights reserved.
 This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
 See the file MOXA-SOFTWARE-NOTICE for details.
 */

#ifndef ACT_SOUTHBOUND_RESULT_HPP
#define ACT_SOUTHBOUND_RESULT_HPP
#include "act_json.hpp"

/**
 * @brief The Southbound result template derives from a variety of result types
 *
 * @tparam T The derives class
 */
template <typename T>
class ActSouthboundResult : public T {
 public:
  /**
   * @brief Construct a new Act Southbound Result object
   *
   */
  ActSouthboundResult() : T() {};

  /**
   * @brief Construct a new Act Southbound Result object
   *
   * @param device_ip
   */
  ActSouthboundResult(QString device_ip) : T(device_ip) {};
};

/**
 * @brief The Southbound result base class
 *
 */
class ActSouthboundResultBase : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_ip, DeviceIp);  ///< The device IP
  // ACT_JSON_FIELD(QString, result_body, ResultBody);                    ///< The qstring result body
  // ACT_JSON_FIELD(QString, error_message, ErrorMessage);                ///< The qstring error message
  // ACT_JSON_FIELD(quint16, error_code, ErrorCode);                      ///< The error code
  // ACT_JSON_ENUM(SouthboundResultStatus, result_status, ResultStatus);  ///< The enum result status

 public:
  /**
   * @brief Construct a new Southbound Result object
   *
   */
  ActSouthboundResultBase() {};

  /**
   * @brief Construct a new Southbound Result object
   *
   * @param device_ip The device IP
   */
  ActSouthboundResultBase(QString device_ip) : device_ip_(device_ip) {};
};

class ActInterfaceIdValue : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, interface_id, InterfaceId);  ///< The InterfaceId item
  ACT_JSON_FIELD(QString, value, Value);              ///< The Value item

 public:
  /**
   * @brief Construct a new Interface id Value object
   *
   */
  ActInterfaceIdValue() {};

  /**
   * @brief Construct a new Interface id Value object
   *
   */
  ActInterfaceIdValue(qint64 interface_id, QString value) : interface_id_(interface_id), value_(value) {};

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActInterfaceIdValue &x) {
    return qHash(x.interface_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActInterfaceIdValue &x, const ActInterfaceIdValue &y) {
    return x.interface_id_ == y.interface_id_;
  }
};

#endif /* ACT_SOUTHBOUND_RESULT_HPP */
