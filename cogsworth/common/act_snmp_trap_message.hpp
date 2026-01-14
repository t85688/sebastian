/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

class ActSnmpTrapMessage : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // ACT_JSON_FIELD(qint64, id, Id);  ///< The ID item

  ACT_JSON_FIELD(QString, ip, IP);                     ///< The IP address of the client
  ACT_JSON_FIELD(QString, oid, OID);                   ///< The OID of the trap message
  ACT_JSON_FIELD(qint64, type, Type);                  ///< The Type of the trap message
  ACT_JSON_FIELD(QString, value, Value);               ///< The Value of the trap message
  ACT_JSON_FIELD(QString, value_object, ValueObject);  ///< The Value of the trap message

 public:
  ActSnmpTrapMessage() {
    this->ip_ = "";
    this->oid_ = "";
    this->type_ = 0;
    this->value_ = "";
  }

  ActSnmpTrapMessage(const QString &ip, const QString &oid, const qint64 &type, const QString &value,
                     const QString &value_object) {
    this->ip_ = ip;
    this->oid_ = oid;
    this->type_ = type;
    this->value_ = value;
    this->value_object_ = value_object;
  }
};
