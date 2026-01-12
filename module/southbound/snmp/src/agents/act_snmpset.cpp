#include "act_snmpset.h"

#include <QDebug>
#include <QString>

#include "act_system.hpp"

ACT_STATUS ActSnmpset::SnmpErrorHandlerWithEntry(const QString &error_fun, const QString &error_reason,
                                                 const ActDevice &device, ActSnmpSetEntry error_entry) {
  ACT_STATUS_INIT();
  qCritical() << QString("%1 %2. Device: %3(%4); Error Entry: %5")
                     .arg(error_fun)
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .arg(error_entry.ToString())
                     .toStdString()
                     .c_str();

  // snmp_free_pdu(response);
  return std::make_shared<ActStatusSouthboundFailed>(error_reason);
}

ACT_STATUS ActSnmpset::SetSnmp(const QList<ActSnmpSetEntry> &set_entry_list, const ActDevice &device) {
  ACT_STATUS_INIT();

  netsnmp_session session;
  void *ss;
  netsnmp_pdu *response;

  // init_snmp("snmpset");
  act_status = BuildSession(session, device, ACT_SNMP_WRITE_TIMEOUT, false);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  // Open an SNMP session
  // For mulit_thread. https://net-snmp.sourceforge.io/docs/README.thread.html
  ss = snmp_sess_open(&session);  // establish the session
  if (!ss) {
    snmp_sess_perror("snmpwalk", &session);
    return SnmpErrorHandler(__func__, "Open SNMP session failed", device);
  }

  // Send each entry
  QList<ActSnmpSetEntry> history_entry_list;  // for debug
  for (auto entry : set_entry_list) {
    // Create the PDU for the set request.
    netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_SET);

    oid oid_array[MAX_OID_LEN];
    size_t oid_array_len = MAX_OID_LEN;
    const char *snmp_oid_char = _strdup(entry.GetOid().toStdString().c_str());

    if (!snmp_parse_oid(snmp_oid_char, oid_array, &oid_array_len)) {  // Check OID
      snmp_free_pdu(response);
      snmp_sess_close(ss);  // close session
      return SnmpErrorHandlerWithEntry(__func__, QString("Parsing OID failed. Error: %1").arg(snmp_oid_char), device,
                                       entry);
    }

    // Bind SNMP request entry
    auto bind_result =
        snmp_add_var(pdu, oid_array, oid_array_len, entry.GetType(), entry.GetValue().toStdString().c_str());
    if (bind_result != 0) {
      snmp_free_pdu(response);
      snmp_sess_close(ss);  // close session
      return SnmpErrorHandlerWithEntry(__func__, QString("Bind SNMP Request failed. Error: %1").arg(snmp_oid_char),
                                       device, entry);
    }

    // Send request
    int status = snmp_sess_synch_response(ss, pdu, &response);
    // STAT_SUCCESS	0, STAT_ERROR	1, STAT_TIMEOUT 2

    if (status != STAT_SUCCESS || ((response != nullptr) && response->errstat != SNMP_ERR_NOERROR)) {
      // Print history entry for debug
      for (auto history_entry : history_entry_list) {
        qDebug() << __func__ << QString("History config Entry: %1").arg(history_entry.ToString()).toStdString().c_str();
      }

      QString err_msg;
      if ((response != nullptr) && response->errstat != SNMP_ERR_NOERROR) {
        err_msg = QString("SNMP status: %1, SNMP errstat: %2(%3)")
                      .arg(status)
                      .arg(snmp_errstring(response->errstat))
                      .arg(response->errstat);
      } else {
        err_msg = QString("SNMP status: %1").arg(status);
      }

      auto snmp_free_pdu(response);
      snmp_sess_close(ss);  // close session
      return SnmpErrorHandlerWithEntry(__func__, QString("Send SNMP Request failed. Error: %1").arg(err_msg), device,
                                       entry);
    }

    // Success
    snmp_free_pdu(response);

    history_entry_list.append(entry);
  }
  snmp_sess_close(ss);  // close session
  return act_status;
}
