/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SNMPSET_H
#define ACT_SNMPSET_H
#include "act_snmp_agent.h"
#include "act_snmp_set_entry.hpp"
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h"
#include "topology/act_device.hpp"

/**
 * @brief The Snmpset module class
 *
 */
class ActSnmpset : public ActSnmpAgent {
 private:
  static ACT_STATUS SnmpErrorHandlerWithEntry(const QString &error_fun, const QString &error_reason,
                                              const ActDevice &device, ActSnmpSetEntry error_entry);

 public:
  static ACT_STATUS SetSnmp(const QList<ActSnmpSetEntry> &set_entry_list, const ActDevice &device);
};

#endif /* ACT_SNMPSET_H */