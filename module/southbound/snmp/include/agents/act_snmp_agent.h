/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SNMP_AGENT_H
#define ACT_SNMP_AGENT_H
#include "act_status.hpp"
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h"
#include "topology/act_device.hpp"

/**
 * @brief The SNMP agent module class
 *
 */
class ActSnmpAgent {
 public:
  static ACT_STATUS BuildSession(netsnmp_session &session, ActDevice device, quint64 timeout, bool readFlag);

  static ACT_STATUS RemoveCacheUser(const ActDevice &device, netsnmp_session &session);

  static QString FindValue(const QString &snmp_value);
  static ACT_STATUS GetSnmpValue(const netsnmp_variable_list *vars, QString &snmp_value);

  static ACT_STATUS SnmpErrorHandler(const QString &error_fun, const QString &error_reason, const ActDevice &device);
};

#endif /* ACT_SNMP_AGENT_H */