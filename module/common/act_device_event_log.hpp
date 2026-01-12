/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"

/**
 * @brief The severity level enum class
 *
 */
enum class ActSeverityLevelEnum { kEmergency, kAlert, kCritical, kError, kWarning, kNotice, kInformational, kDebug };

/**
 * @brief The QMap for severity level enum mapping
 *
 */
static const QMap<QString, ActSeverityLevelEnum> kActSeverityLevelEnumMap = {
    {"Emergency", ActSeverityLevelEnum::kEmergency},
    {"Alert", ActSeverityLevelEnum::kAlert},
    {"Critical", ActSeverityLevelEnum::kCritical},
    {"Error", ActSeverityLevelEnum::kError},
    {"Warning", ActSeverityLevelEnum::kWarning},
    {"Notice", ActSeverityLevelEnum::kNotice},
    {"Informational", ActSeverityLevelEnum::kInformational},
    {"Debug", ActSeverityLevelEnum::kDebug}};

class ActDeviceEventLogEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, bootup_number, BootupNumber);
  // ACT_JSON_FIELD(quint8, severity, Severity);
  ACT_JSON_ENUM(ActSeverityLevelEnum, severity, Severity);
  ACT_JSON_FIELD(QString, timestamp, Timestamp);
  ACT_JSON_FIELD(QString, uptime, Uptime);
  ACT_JSON_FIELD(QString, message, Message);

  // ACT_JSON_FIELD(QString, host_name, HostName);
  // ACT_JSON_FIELD(QString, prog_name, ProgName);
};

/**
 * @brief The Device's Event log
 *
 */
class ActDeviceEventLog : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(quint32, totalnum, TotalNum);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActDeviceEventLogEntry, entries, Entries);
};
