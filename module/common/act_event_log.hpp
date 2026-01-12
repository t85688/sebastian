/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"
/**
 * @brief The response body data of event logs
 *
 */
class ActEventLogData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, code, code);
  ACT_JSON_FIELD(QString, timestamp, timestamp);
  ACT_JSON_FIELD(qint32, id, id);
  ACT_JSON_FIELD(QString, message, message);
  ACT_JSON_FIELD(QString, source, source);
  ACT_JSON_FIELD(QString, sourceIp, sourceIp);
  ACT_JSON_FIELD(QString, severity, severity);
  ACT_JSON_COLLECTION(QList, QString, variables, variables);
};

/**
 * @brief The response of event logs
 *
 */
class ActEventLogsResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, count, count);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActEventLogData, data, data);
  ACT_JSON_FIELD(qint32, limit, limit);
  ACT_JSON_FIELD(qint32, offset, offset);
  ACT_JSON_FIELD(qint32, total, total);
};

/**
 * @brief The ACT syslog query data
 *
 */
class ActSyslogQueryData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, limit, Limit);
  ACT_JSON_FIELD(qint32, offset, Offset);
  ACT_JSON_FIELD(QString, severities, Severities);
  ACT_JSON_FIELD(QString, facilities, Facilities);
  ACT_JSON_FIELD(QString, starttime, Starttime);
  ACT_JSON_FIELD(QString, endtime, Endtime);
  ACT_JSON_FIELD(QString, ipaddress, Ipaddress);

 public:
  /**
   * @brief Construct a new Act syslog query data object
   *
   */
  ActSyslogQueryData(qint32 limit, qint32 offset) {
    this->limit_ = limit;
    this->offset_ = offset;
    this->severities_ = "-1";
    this->facilities_ = "-1";
    this->starttime_ = "-1";
    this->endtime_ = "-1";
    this->ipaddress_ = "-1";
  }
};

/**
 * @brief The response body data of syslog
 *
 */
class ActSyslogData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, id, id);
  ACT_JSON_FIELD(qint32, severity, severity);
  ACT_JSON_FIELD(qint32, facility, facility);
  ACT_JSON_FIELD(QString, ipaddress, ipaddress);
  ACT_JSON_FIELD(QString, syslog_time, syslogTime);
  ACT_JSON_FIELD(QString, timestamp, timestamp);
  ACT_JSON_FIELD(QString, message, message);
};

/**
 * @brief The response of syslog
 *
 */
class ActSyslogsResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, count, count);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActSyslogData, data, data);
  ACT_JSON_FIELD(qint32, limit, limit);
  ACT_JSON_FIELD(qint32, offset, offset);
  ACT_JSON_FIELD(qint32, total, total);
};

class ActCsvFilepath : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, filepath, filepath);
};

class ActDeleteSyslogsResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, message, message);
};

class ActSyslogPutConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
};

class ActSyslogGetConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enable, enable);
  ACT_JSON_FIELD(qint32, port, port);
};