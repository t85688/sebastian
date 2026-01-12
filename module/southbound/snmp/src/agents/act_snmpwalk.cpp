#include "act_snmpwalk.h"

#include <QDebug>
#include <QString>

#include "act_system.hpp"

ACT_STATUS ActSnmpwalk::GetSnmpList(const QList<QString> &oid_list, const ActDevice &device,
                                    ActSnmpResult<ActSnmpMessageMap> &snmp_result) {
  ACT_STATUS_INIT();

  QMap<QString, QString> snmp_message_map;

  oid oid_array[MAX_OID_LEN];
  size_t oid_array_len = MAX_OID_LEN;
  netsnmp_session session;
  void *ss;
  netsnmp_pdu *response;

  // init_snmp("snmpwalk");
  act_status = BuildSession(session, device, ACT_SNMP_READ_TIMEOUT, true);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Open an SNMP session
  // For multi_thread. https://net-snmp.sourceforge.io/docs/README.thread.html
  ss = snmp_sess_open(&session);  // establish the session
  if (!ss) {
    snmp_sess_perror("snmpwalk", &session);
    return SnmpErrorHandler(__func__, "Open SNMP session failed", device);
  }

  for (auto snmp_oid : oid_list) {
    char *snmp_oid_char = _strdup(snmp_oid.toStdString().c_str());

    // Check OID is correctly
    if (!snmp_parse_oid(snmp_oid_char, oid_array, &oid_array_len)) {
      qCritical() << __func__ << "Parsing OID failed. OID:" << snmp_oid << "Error:";
      snmp_perror(snmp_oid_char);
      snmp_sess_close(ss);  // close session
      return std::make_shared<ActStatusInternalError>("SNMP");
    }

    // Create the PDU for the data for our request.
    netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GET);
    snmp_add_null_var(pdu, oid_array, oid_array_len);
    // qDebug() << "Snmpwalk::GetSnmp(): Send Request:" << OidToString(oid_array, oid_array_len);

    // Send the Request out.
    int status = snmp_sess_synch_response(ss, pdu, &response);

    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
      // Get Message
      act_status = GetSnmpMessageGet(response, oid_array, oid_array_len, snmp_message_map);
      if (act_status->GetStatus() == ActStatusType::kInternalError) {  // include SKIP(no data) & FAILED
        qCritical() << __func__
                    << QString("Device(%1) GetSnmpList > GetSnmpMessageGet() failed. Request: %2.")
                           .arg(device.GetIpv4().GetIpAddress())
                           .arg(OidToString(oid_array, oid_array_len))
                           .toStdString()
                           .c_str();
        snmp_sess_close(ss);  // close session
        return act_status;
      }
    } else {
      qCritical() << __func__
                  << QString("Device(%1) GetSnmpList() failed. Request: %2")
                         .arg(device.GetIpv4().GetIpAddress())
                         .arg(OidToString(oid_array, oid_array_len))
                         .toStdString()
                         .c_str();
      if (status == STAT_SUCCESS) {  // response->errstat != SNMP_ERR_NOERROR
        qCritical() << ", snmp_synch_response() failed. In packet Reason:" << snmp_errstring(response->errstat);
      } else if (status == STAT_TIMEOUT) {
        qCritical() << ", snmp_synch_response() failed. Timeout(No response)";
      } else {  // STAT_ERROR
        qCritical() << ", snmp_synch_response() failed. Session error.";
      }
      snmp_sess_close(ss);  // close session
      return std::make_shared<ActStatusInternalError>("SNMP");
    }
  }

  snmp_sess_close(ss);  // close session
  snmp_result.SetSnmpMessage(snmp_message_map);
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSnmpwalk::GetSnmpSubTreeByBulk(const QString &snmp_oid, const ActDevice &device,
                                             ActSnmpResult<ActSnmpMessageMap> &snmp_result) {
  ACT_STATUS_INIT();
  QMap<QString, QString> snmp_message_map;

  oid oid_array[MAX_OID_LEN];
  oid next_oid_array[MAX_OID_LEN];
  size_t oid_array_len = MAX_OID_LEN;
  size_t next_oid_array_len = MAX_OID_LEN;
  const char *snmp_oid_char = _strdup(snmp_oid.toStdString().c_str());
  bool running = true;
  bool first_get = true;
  netsnmp_session session;
  void *ss;
  netsnmp_pdu *response;

  // init_snmp("snmpwalk");
  act_status = BuildSession(session, device, ACT_SNMP_READ_BULK_TIMEOUT, true);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate OID
  if (!snmp_parse_oid(snmp_oid_char, oid_array, &oid_array_len)) {
    qCritical() << __func__ << "Parsing OID failed. OID:" << snmp_oid << "Error:";
    snmp_perror(snmp_oid_char);
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  if (!snmp_parse_oid(snmp_oid_char, next_oid_array, &next_oid_array_len)) {
    qCritical() << __func__ << "Parsing OID failed. OID:" << snmp_oid << "Error:";
    snmp_perror(snmp_oid_char);
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  // Open an SNMP session
  // For multi_thread. https://net-snmp.sourceforge.io/docs/README.thread.html
  ss = snmp_sess_open(&session);  // establish the session
  if (!ss) {
    snmp_sess_perror("snmpwalk", &session);
    return SnmpErrorHandler(__func__, "Open SNMP session failed", device);
  }

  while (running) {
    // Create the PDU for the data for our request.
    netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
    pdu->non_repeaters = 0;
    pdu->max_repetitions = 5;

    snmp_add_null_var(pdu, next_oid_array, next_oid_array_len);
    // Send the Request out.
    int status = snmp_sess_synch_response(ss, pdu, &response);

    // STAT_SUCCESS	0, STAT_ERROR	1, STAT_TIMEOUT 2
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
      // Get Message
      bool get_next_message = true;
      act_status = GetSnmpMessageBulk(response, oid_array, oid_array_len, next_oid_array, next_oid_array_len, first_get,
                                      snmp_message_map, get_next_message);
      if (!IsActStatusSuccess(act_status)) {  // include SKIP(no data) & FAILED
        if (act_status->GetStatus() == ActStatusType::kSkip) {
          // qDebug() << __func__
          //          << QString("Device(%1) Skip request").arg(device.GetIpv4().GetIpAddress()).toStdString().c_str();
          break;
        }
        qCritical() << __func__
                    << QString("Device(%1) GetSnmpSubTreeByBulk > GetSnmpMessageBulk() failed. Request: %2.")
                           .arg(device.GetIpv4().GetIpAddress())
                           .arg(OidToString(next_oid_array, next_oid_array_len))
                           .toStdString()
                           .c_str();

        snmp_sess_close(ss);  // close session
        return act_status;
      } else {  // get message success
        if (get_next_message) {
          running = true;
          first_get = false;
        } else {
          running = false;
        }
      }

    } else {
      running = false;
      qCritical() << __func__
                  << QString("Device(%1) GetSnmpSubTreeByBulk() failed. Request: %2")
                         .arg(device.GetIpv4().GetIpAddress())
                         .arg(OidToString(next_oid_array, next_oid_array_len))
                         .toStdString()
                         .c_str();

      if (status == STAT_SUCCESS) {  // response->errstat != SNMP_ERR_NOERROR
        qCritical() << ", snmp_synch_response() failed. In packet Reason:" << snmp_errstring(response->errstat);
      } else if (status == STAT_TIMEOUT) {
        qCritical() << ", snmp_synch_response() failed. Timeout(No response)";
      } else {  // STAT_ERROR
        qCritical() << ", snmp_synch_response() failed. Session error.";
        qDebug() << QString("Device(%1) SNMP(OID: %2) status: %3")
                        .arg(device.GetIpv4().GetIpAddress())
                        .arg(snmp_oid)
                        .arg(status)
                        .toStdString()
                        .c_str();
        if (response) {
          qDebug() << QString("Device(%1) SNMP(OID: %2) response->errstat: %3")
                          .arg(device.GetIpv4().GetIpAddress())
                          .arg(snmp_oid)
                          .arg(response->errstat)
                          .toStdString()
                          .c_str();
        }
      }
      snmp_sess_close(ss);  // close session
      return std::make_shared<ActStatusInternalError>("SNMP");
    }
  }
  snmp_sess_close(ss);  // close session
  snmp_result.SetSnmpMessage(snmp_message_map);

  // qDebug() << QString("Device(%1) SNMP(OID: %2) response size: %3")
  //                 .arg(device.GetIpv4().GetIpAddress())
  //                 .arg(snmp_oid)
  //                 .arg(snmp_message_map.size())
  //                 .toStdString()
  //                 .c_str();
  // for (auto key : snmp_message_map.keys()) {
  //   qDebug() << QString("Device(%1) SNMP response: %2: %3")
  //                   .arg(device.GetIpv4().GetIpAddress())
  //                   .arg(key)
  //                   .arg(snmp_message_map[key])
  //                   .toStdString()
  //                   .c_str();
  // }
  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActSnmpwalk::GetSnmpSubTree(const QString &snmp_oid, const ActDevice &device,
                                       ActSnmpResult<ActSnmpMessageMap> &snmp_result) {
  ACT_STATUS_INIT();
  QMap<QString, QString> snmp_message_map;

  oid oid_array[MAX_OID_LEN];
  oid next_oid_array[MAX_OID_LEN];
  size_t oid_array_len = MAX_OID_LEN;
  size_t next_oid_array_len = MAX_OID_LEN;
  const char *snmp_oid_char = _strdup(snmp_oid.toStdString().c_str());
  bool running = true;
  bool first_get = true;
  netsnmp_session session;
  void *ss;
  netsnmp_pdu *response;

  // init_snmp("snmpwalk");
  act_status = BuildSession(session, device, ACT_SNMP_READ_TIMEOUT, true);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Generate OID
  if (!snmp_parse_oid(snmp_oid_char, oid_array, &oid_array_len)) {
    qCritical() << __func__ << "Parsing OID failed. OID:" << snmp_oid << "Error:";
    snmp_perror(snmp_oid_char);
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  // Generate next OID
  QString next_snmp_oid = GetNextOid(snmp_oid);
  if (next_snmp_oid.isEmpty()) {
    qCritical() << __func__ << "Get next OID fail";
    return std::make_shared<ActStatusInternalError>("SNMP");
  }
  const char *next_snmp_oid_char = _strdup(next_snmp_oid.toStdString().c_str());

  if (!snmp_parse_oid(next_snmp_oid_char, next_oid_array, &next_oid_array_len)) {
    qCritical() << __func__ << "Parsing next OID failed. OID:" << snmp_oid << "Error:";
    snmp_perror(next_snmp_oid_char);
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  // Open an SNMP session
  // For multi_thread. https://net-snmp.sourceforge.io/docs/README.thread.html
  ss = snmp_sess_open(&session);  // establish the session
  if (!ss) {
    snmp_sess_perror("snmpwalk", &session);
    return SnmpErrorHandler(__func__, "Open SNMP session failed", device);
  }

  while (running) {
    // Create the PDU for the data for our request.
    netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    snmp_add_null_var(pdu, oid_array, oid_array_len);
    // qDebug() << "Snmpwalk::GetSnmpSubTree(): Send Request:" << OidToString(oid_array, oid_array_len);

    // Send the Request out.
    int status = snmp_sess_synch_response(ss, pdu, &response);

    // STAT_SUCCESS	0, STAT_ERROR	1, STAT_TIMEOUT 2
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
      bool get_next_message = true;
      act_status = GetSnmpMessageNext(response, oid_array, oid_array_len, next_oid_array, next_oid_array_len, first_get,
                                      snmp_message_map, get_next_message);

      if (!IsActStatusSuccess(act_status)) {  // include SKIP(no data) & FAILED
        if (act_status->GetStatus() == ActStatusType::kSkip) {
          break;
        }
        qCritical() << __func__
                    << QString("Device(%1) GetSnmpSubTree > GetSnmpMessageNext() failed. Request: %2.")
                           .arg(device.GetIpv4().GetIpAddress())
                           .arg(OidToString(oid_array, oid_array_len))
                           .toStdString()
                           .c_str();

        snmp_sess_close(ss);  // close session
        return act_status;
      } else {
        if (get_next_message) {
          running = true;
          first_get = false;
        } else {
          running = false;
        }
      }

    } else {
      running = false;
      qCritical() << __func__
                  << QString("Device(%1) GetSnmpSubTree() failed. Request: %2")
                         .arg(device.GetIpv4().GetIpAddress())
                         .arg(OidToString(oid_array, oid_array_len))
                         .toStdString()
                         .c_str();
      if (status == STAT_SUCCESS) {  // response->errstat != SNMP_ERR_NOERROR
        qCritical() << "snmp_synch_response() failed. In packet Reason:" << snmp_errstring(response->errstat);
      } else if (status == STAT_TIMEOUT) {
        qCritical() << "snmp_synch_response() failed. Timeout(No response)";
      } else {  // STAT_ERROR
        qCritical() << "snmp_synch_response() failed. Session error.";
      }
      snmp_sess_close(ss);  // close session
      return std::make_shared<ActStatusInternalError>("SNMP");
    }
  }
  snmp_sess_close(ss);  // close session
  snmp_result.SetSnmpMessage(snmp_message_map);
  return ACT_STATUS_SUCCESS;
}

QString ActSnmpwalk::GetNextOid(const QString &snmp_oid) {
  QString ori_oid = snmp_oid;
  quint32 last_value = ori_oid.section('.', -1).toUInt() + 1;
  QString end_oid = ori_oid.section('.', 0, -2) + '.' + QString::number(last_value);

  return end_oid;
}

QString ActSnmpwalk::OidToString(oid *oid_array, size_t oid_array_len) {
  QString str;
  for (int i = 0; i < oid_array_len - 1; i++) {
    str.append(QString::number(oid_array[i]));
    str.append(".");
  }
  str.append(QString::number(oid_array[oid_array_len - 1]));
  return str;
}

bool ActSnmpwalk::IsSubtree(const QString &base_oid, const QString &sub_oid) {
  // Check sub_oid is base_oid prefix
  if (sub_oid.startsWith(base_oid)) {
    // Check OID last is '.' or equal
    if (sub_oid.length() == base_oid.length() || sub_oid.mid(base_oid.length(), 1) == ".") {
      return true;
    }
  }
  return false;
}

ACT_STATUS ActSnmpwalk::GetSnmpMessageNext(const netsnmp_pdu *response, oid *oid_array, size_t &oid_array_len,
                                           oid *next_oid_array, size_t next_oid_array_len, const bool &first_get,
                                           QMap<QString, QString> &snmp_message_map_result,
                                           bool &result_get_next_message) {
  ACT_STATUS_INIT();

  result_get_next_message = false;
  // Handle response's entries
  netsnmp_variable_list *vars;
  for (vars = response->variables; vars; vars = vars->next_variable) {
    // print_variable(vars->name, vars->name_length, vars);
    // qDebug() << "Snmpwalk::GetSnmpMessageNext: Response's OID:" << OidToString(vars->name, vars->name_length);

    // snmp_oid_compare() return:: -1: name1 < name2, 0 :name1 = name2, 1 :name1 > name2
    auto oid_compare_result = snmp_oid_compare(next_oid_array, next_oid_array_len, vars->name, vars->name_length);
    if (oid_compare_result <= 0) {  // not part of this oid's subtree
      if (first_get) {
        // qDebug() << __func__ << "No data available!(OID not subtree)";
        return std::make_shared<ActStatusBase>(ActStatusType::kSkip, ActSeverity::kDebug);
        // qCritical() << __func__ << "Error: The OID not subtree. OID:" << OidToString(oid_array, oid_array_len)
        //             << "Next OID:" << OidToString(next_oid_array, next_oid_array_len)
        //             << "Response OID:" << OidToString(vars->name, vars->name_length) << "Response type:" <<
        //             vars->type
        //             << "result:" << oid_compare_result;

        // return std::make_shared<ActStatusInternalError>("SNMP");
      }
      break;
    }
    // Check vars type
    if (vars->type == SNMP_ENDOFMIBVIEW) {
      if (first_get) {
        // qDebug() << __func__ << "No data available!(2)";
        return std::make_shared<ActStatusBase>(ActStatusType::kSkip, ActSeverity::kDebug);
      }
    } else if ((vars->type == SNMP_NOSUCHOBJECT) || (vars->type == SNMP_NOSUCHINSTANCE)) {
      // Get the exception value

      qCritical() << __func__ << __func__ << "Get the exception value.";
      return std::make_shared<ActStatusInternalError>("SNMP");
    } else {  // process the value

      // Check response OID.
      if (snmp_oid_compare(oid_array, oid_array_len, vars->name, vars->name_length) >= 0) {
        qCritical() << __func__ << __func__ << "Error: OID not increased:" << OidToString(vars->name, vars->name_length)
                    << ">=" << OidToString(oid_array, oid_array_len);
        return std::make_shared<ActStatusInternalError>("SNMP");
      }

      // Start process response value
      QString snmp_value = "";
      act_status = GetSnmpValue(vars, snmp_value);
      if (act_status->GetStatus() == ActStatusType::kSkip) {  // SKIP
        // qDebug() << __func__ << "No data available!(3)";
        return act_status;
      }

      if (act_status->GetStatus() == ActStatusType::kInternalError) {
        qCritical() << __func__ << "GetSnmpValue() failed.";
        return act_status;
      }

      // qDebug() << "Snmpwalk::GetSnmpMessageNext: Response value:" << snmp_value;
      result_get_next_message = true;
      snmp_message_map_result.insert(OidToString(vars->name, vars->name_length), snmp_value);
      memmove((char *)oid_array, (char *)vars->name, vars->name_length * sizeof(oid));  // copy next_oid to oid_array
      oid_array_len = vars->name_length;
    }
  }
  return act_status;
}

ACT_STATUS ActSnmpwalk::GetSnmpMessageGet(const netsnmp_pdu *response, oid *oid_array, size_t &oid_array_len,
                                          QMap<QString, QString> &snmp_message_map_result) {
  ACT_STATUS_INIT();

  // Handle response's entries
  netsnmp_variable_list *vars;
  for (vars = response->variables; vars; vars = vars->next_variable) {
    // qDebug() << "Snmpwalk::GetSnmpMessageGet: Response's OID:" << OidToString(vars->name, vars->name_length);

    // Compare requset's oid and response's oid
    // snmp_oid_compare() return:: -1: name1 < name2, 0 :name1 = name2, 1 :name1 > name2
    if (snmp_oid_compare(oid_array, oid_array_len, vars->name, vars->name_length) != 0) {  // not same
      qCritical() << __func__ << "Get the response's oid not same as the request's oid.";
      return std::make_shared<ActStatusInternalError>("SNMP");
    }

    if ((vars->type == SNMP_NOSUCHOBJECT) || (vars->type == SNMP_NOSUCHINSTANCE)) {
      // Get the exception value

      qCritical() << __func__ << "Get the exception value.";
      return std::make_shared<ActStatusInternalError>("SNMP");
    } else {  // process the value
      QString snmp_value = "";
      act_status = GetSnmpValue(vars, snmp_value);
      if (act_status->GetStatus() == ActStatusType::kSkip) {  // SKIP
        // qDebug() << "No data available!(3)";
        return act_status;
      }

      if (act_status->GetStatus() == ActStatusType::kInternalError) {
        qCritical() << __func__ << "GetSnmpValue() failed.";
        return act_status;
      }

      // qDebug() << "Snmpwalk::GetSnmpMessageGet: Response value:" << snmp_value;
      snmp_message_map_result.insert(OidToString(vars->name, vars->name_length), snmp_value);
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpwalk::GetSnmpMessageBulk(const netsnmp_pdu *response, oid *oid_array, size_t &oid_array_len,
                                           oid *next_oid_array, size_t &next_oid_array_len, const bool &first_get,
                                           QMap<QString, QString> &snmp_message_map_result,
                                           bool &result_get_next_message) {
  ACT_STATUS_INIT();
  result_get_next_message = true;

  // Handle response's entries
  netsnmp_variable_list *vars;

  int vars_count = 0;
  auto req_oid_str = OidToString(oid_array, oid_array_len);
  // qDebug() << __func__ << "Request: OID:" << req_oid_str;

  for (vars = response->variables; vars; vars = vars->next_variable) {
    vars_count++;
    // Check response var's OID is the request OID subtree
    auto reply_oid_str = OidToString(vars->name, vars->name_length);
    if (!IsSubtree(req_oid_str, reply_oid_str)) {
      // not request subtree would stop handle remain vars
      result_get_next_message = false;
      break;
    }

    // Check response var's OID not duplicated
    if (snmp_message_map_result.contains(reply_oid_str)) {
      // stop handle remain vars
      result_get_next_message = false;
      // qDebug() << __func__ << "Response OID duplicated: OID:" << reply_oid_str;
      break;
    }

    if ((vars->type == SNMP_NOSUCHOBJECT) || (vars->type == SNMP_NOSUCHINSTANCE)) {
      // Get the exception value
      result_get_next_message = false;

      qCritical() << __func__ << "Get the exception value.";
      return std::make_shared<ActStatusInternalError>("SNMP");
    } else {  // process the value
      QString snmp_value = "";
      act_status = GetSnmpValue(vars, snmp_value);
      if (act_status->GetStatus() == ActStatusType::kSkip) {  // SKIP
        result_get_next_message = false;
        // qDebug() << "No data available!(3)";
        return act_status;
      }

      if (act_status->GetStatus() == ActStatusType::kInternalError) {
        result_get_next_message = false;
        qCritical() << __func__ << "GetSnmpValue() failed.";
        return act_status;
      }

      // qDebug() << __func__ << "Response: OID:" << reply_oid_str << "Value:" << snmp_value;
      snmp_message_map_result.insert(reply_oid_str, snmp_value);

      // Set next_oid
      memmove((char *)next_oid_array, (char *)vars->name, vars->name_length * sizeof(oid));
      next_oid_array_len = vars->name_length;
    }
  }

  if (vars_count == 0) {
    result_get_next_message = false;
    if (first_get) {
      // qDebug() << __func__ << "No data available!(2)";
      return std::make_shared<ActStatusBase>(ActStatusType::kSkip, ActSeverity::kDebug);
    }
  }

  auto next_req_oid_str = OidToString(next_oid_array, next_oid_array_len);
  // qDebug() << __func__ << "next_req_oid_str:" << next_req_oid_str;
  return act_status;
}
