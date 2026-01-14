/* Copyright (C) MOXA Inc. All rights reserved.
 This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
 See the file MOXA-SOFTWARE-NOTICE for details.
 */

#ifndef ACT_SNMP_SNMP_SET_ENTRY_HPP
#define ACT_SNMP_SNMP_SET_ENTRY_HPP
#include "act_json.hpp"

class ActSnmpSetEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, oid, Oid);      ///< SNMP oid item
  ACT_JSON_FIELD(QString, value, Value);  ///< SNMP value item
  ACT_JSON_FIELD(char, type, Type);       ///< SNMP type item
 public:
  ActSnmpSetEntry() : type_('n') {};
  ActSnmpSetEntry(const QString &oid, const QString &value, const char &type)
      : oid_(oid), value_(value), type_(type) {};
};
#endif /* ACT_SNMP_SNMP_SET_ENTRY_HPP */