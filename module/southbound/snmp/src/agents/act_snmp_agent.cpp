#include "act_snmp_agent.h"

#include <QDebug>
#include <QString>
#include <QtGlobal>
ACT_STATUS ActSnmpAgent::BuildSession(netsnmp_session &session, ActDevice device, quint64 timeout, bool readFlag) {
  ACT_STATUS_INIT();

  try {
    snmp_sess_init(&session);                /* set up defaults */
    session.timeout = timeout;               // (library default = 5 second)
    session.retries = ACT_SNMP_RETRY_TIMES;  // number of retries before timeout(library default = 5)
    QString peername =
        QString("%1:%2").arg(device.GetIpv4().GetIpAddress()).arg(device.GetSnmpConfiguration().GetPort());
    session.peername = _strdup(peername.toStdString().c_str());
    ActSnmpVersionEnum versionEnum = device.GetSnmpConfiguration().GetVersion();

    // SNMPv1, SNMPv2c
    if (versionEnum == ActSnmpVersionEnum::kV1 || versionEnum == ActSnmpVersionEnum::kV2c) {
      session.version = (versionEnum == ActSnmpVersionEnum::kV1) ? SNMP_VERSION_1 : SNMP_VERSION_2c;
      session.community =
          readFlag ? (u_char *)_strdup(device.GetSnmpConfiguration().GetReadCommunity().toStdString().c_str())
                   : (u_char *)_strdup(device.GetSnmpConfiguration().GetWriteCommunity().toStdString().c_str());
      session.community_len = strlen((const char *)session.community);
    }

    // SNMPv3
    if (versionEnum == ActSnmpVersionEnum::kV3) {
      session.version = SNMP_VERSION_3;
      session.securityName = _strdup(device.GetSnmpConfiguration().GetUsername().toStdString().c_str());
      session.securityNameLen = strlen(session.securityName);

      // [bugfix:3349] Some combinations of SNMP V3 cannot connect to the device
      // ref: https://stackoverflow.com/questions/18380435/net-snmp-is-not-changing-auth-and-priv-protocol-correctly
      // Remove the Cache user
      netsnmp_session remove_cache_session = session;
      act_status = RemoveCacheUser(device, remove_cache_session);
      if (!IsActStatusSuccess(act_status)) {
        return act_status;
      }

      // None Authentication type
      if (device.GetSnmpConfiguration().GetAuthenticationType() == ActSnmpAuthenticationTypeEnum::kNone) {
        session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;
        return act_status;
      }

      // Authentication type
      if (device.GetSnmpConfiguration().GetAuthenticationType() == ActSnmpAuthenticationTypeEnum::kMD5) {
        // MD5
        session.securityAuthProto = usmHMACMD5AuthProtocol;
        session.securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
      } else {
        // SHA-1
        session.securityAuthProto = usmHMACSHA1AuthProtocol;
        session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
      }

      // Generate the SecurityAuthKey
      auto auth_key = device.GetSnmpConfiguration().GetAuthenticationPassword();
      session.securityAuthKeyLen = USM_AUTH_KU_LEN;
      if (generate_Ku(session.securityAuthProto, session.securityAuthProtoLen, (u_char *)auth_key.toStdString().c_str(),
                      static_cast<u_int>(strlen(auth_key.toStdString().c_str())), session.securityAuthKey,
                      &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
        qCritical() << __func__ << "Generate the SecurityAuthKey failed.";
        return std::make_shared<ActStatusInternalError>("Generate the SecurityAuthKey failed");
      }

      // None Encryption type
      if (device.GetSnmpConfiguration().GetDataEncryptionType() == ActSnmpDataEncryptionTypeEnum::kNone) {
        session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        return act_status;
      }

      // Has Encryption type
      session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;

      // Encryption type
      if (device.GetSnmpConfiguration().GetDataEncryptionType() == ActSnmpDataEncryptionTypeEnum::kDES) {
        // DES
        session.securityPrivProto = usmDESPrivProtocol;
        session.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
      } else {
        // AES
        session.securityPrivProto = usmAESPrivProtocol;
        session.securityPrivProtoLen = USM_PRIV_PROTO_AES_LEN;
      }

      // Generate the SecurityPrivKey
      QString priv_key = device.GetSnmpConfiguration().GetDataEncryptionKey();
      session.securityPrivKeyLen = USM_PRIV_KU_LEN;
      if (generate_Ku(session.securityAuthProto, session.securityAuthProtoLen, (u_char *)priv_key.toStdString().c_str(),
                      static_cast<u_int>(strlen(priv_key.toStdString().c_str())), session.securityPrivKey,
                      &session.securityPrivKeyLen) != SNMPERR_SUCCESS) {
        qCritical() << __func__ << "Generate the SecurityPrivKey failed.";
        return std::make_shared<ActStatusInternalError>("Generate the SecurityPrivKey failed");
      }
    }

  } catch (std::exception &e) {
    qCritical() << "ActSnmpAgent::BuildSession() failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  return act_status;
}

ACT_STATUS ActSnmpAgent::RemoveCacheUser(const ActDevice &device, netsnmp_session &session) {
  ACT_STATUS_INIT();

  // Get DUT EngineID
  QString session_engine_id;
  netsnmp_pdu *response;
  netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GET);
  void *ss;
  ss = snmp_sess_open(&session);  // establish the session
  if (!ss) {
    snmp_sess_perror("snmpwalk", &session);
    return SnmpErrorHandler(__func__, "Open SNMP session failed", device);
  }

  snmp_sess_synch_response(ss, pdu, &response);
  if (response) {
    if (response->contextEngineID && response->contextEngineIDLen > 0) {
      for (size_t i = 0; i < response->contextEngineIDLen; i++) {
        session_engine_id += QString::number(response->contextEngineID[i], 16).rightJustified(2, '0').toUpper();
      }
    }
    snmp_free_pdu(response);
  }

  // Remove cache user
  if (!session_engine_id.isEmpty()) {
    usmUser *usm_user = usm_get_userList();
    while (usm_user != NULL) {
      usmUser *dummy = usm_user;
      QString usm_sec_name = QString::fromUtf8(usm_user->secName);
      QString usm_engine_id;
      for (size_t i = 0; i < usm_user->engineIDLen; i++) {
        usm_engine_id += QString::number(usm_user->engineID[i], 16).rightJustified(2, '0').toUpper();
      }

      if ((session_engine_id == usm_engine_id) && (device.GetSnmpConfiguration().GetUsername() == usm_sec_name)) {
        usm_remove_user(usm_user);
        break;
      }
      usm_user = dummy->next;
    }
  }

  return act_status;
}

ACT_STATUS ActSnmpAgent::GetSnmpValue(const netsnmp_variable_list *vars, QString &snmp_value) {
  ACT_STATUS_INIT();

  try {
    // qDebug() << "vars->type:" << vars->type;
    size_t buf_len = 256;
    switch (vars->type) {
      case ASN_OCTET_STR:
        // snmp_value = QString::fromUtf8(reinterpret_cast<const char *>(vars->val.string), vars->val_len);
        char *buf;
        if ((buf = (char *)calloc(buf_len, 1)) == NULL) {
          return std::make_shared<ActStatusBase>(ActStatusType::kSkip, ActSeverity::kDebug);
        } else {
          if (snprint_value(buf, buf_len, vars->name, vars->name_length, vars)) {
            for (int i = 0; buf[i] != '\0'; i++) {
              snmp_value.push_back(buf[i]);
            }
            snmp_value = FindValue(snmp_value);
          } else {
            snmp_value = reinterpret_cast<char *>(vars->val.string);
          }
        }
        SNMP_FREE(buf);

        break;
      case ASN_OBJECT_ID:
        char oid_str[SPRINT_MAX_LEN];
        snprint_objid(oid_str, sizeof(oid_str), vars->val.objid, vars->val_len / sizeof(oid));
        snmp_value = QString(oid_str);
        if (snmp_value.contains("iso")) {
          snmp_value.replace("iso", "1");
        }
        break;
      case ASN_IPADDRESS:
        snmp_value = QString("%1.%2.%3.%4")
                         .arg(vars->val.string[0])
                         .arg(vars->val.string[1])
                         .arg(vars->val.string[2])
                         .arg(vars->val.string[3]);
        break;
      case ASN_COUNTER64: {
        struct counter64 counter = *vars->val.counter64;
        quint64 value = (static_cast<quint64>(counter.high) << 32) | counter.low;
        snmp_value = QString::number(value);
      } break;
      case ASN_INTEGER:
      case ASN_INTEGER64:
      case ASN_COUNTER:
      case ASN_GAUGE:
      case ASN_TIMETICKS:
      case ASN_UINTEGER:
      case ASN_UNSIGNED64:
        snmp_value = QString::number(*vars->val.integer);
        break;
      default:
        snmp_value = "";
    }

  } catch (std::exception &e) {
    qCritical() << __func__ << "GetSnmpValue failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("SNMP");
  }

  return act_status;
}

QString ActSnmpAgent::FindValue(const QString &snmp_value) {
  QString value = snmp_value;
  auto pos = value.indexOf("STRING:");
  if (pos != -1) {
    value.remove(0, pos + 7);
  }
  value = value.remove('"').trimmed();
  return value;
}

ACT_STATUS ActSnmpAgent::SnmpErrorHandler(const QString &error_fun, const QString &error_reason,
                                          const ActDevice &device) {
  ACT_STATUS_INIT();
  qCritical() << QString("%1 %2. Device: %3(%4)")
                     .arg(error_fun)
                     .arg(error_reason)
                     .arg(device.GetIpv4().GetIpAddress())
                     .arg(device.GetId())
                     .toStdString()
                     .c_str();

  // snmp_free_pdu(response);
  return std::make_shared<ActStatusSouthboundFailed>(error_reason);
}
