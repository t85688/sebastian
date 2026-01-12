/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SNMPWALK_H
#define ACT_SNMPWALK_H

#include "act_snmp_agent.h"
#include "act_snmp_result.hpp"
#include "act_status.hpp"
#include "net-snmp/net-snmp-config.h"
#include "net-snmp/net-snmp-includes.h"
#include "topology/act_device.hpp"
#define ASN_GAUGE32 ((u_char)0x42)

// https://net-snmp.sourceforge.io/dev/agent/group__library.html
// ttps://net-snmp.sourceforge.io/dev/agent/group__library.html#var-members
// For mib parsing:
// https://net-snmp.sourceforge.io/dev/agent/group__mib__utilities.html

/**
 * @brief The Snmpwalk module class
 *
 */
class ActSnmpwalk : public ActSnmpAgent {
 private:
  /**
   * @brief Get the Next Oid object
   *
   * @param snmp_oid
   * @return QString
   */
  static QString GetNextOid(const QString &snmp_oid);

  /**
   * @brief Transfer the oid pointer to string
   *
   * @param oid_array
   * @param oid_array_len
   * @return QString
   */
  static QString OidToString(oid *oid_array, size_t oid_array_len);

  /**
   * @brief Compare the OID is subtree
   *
   * @param base_oid
   * @param sub_oid
   * @return true
   * @return false
   */
  static bool IsSubtree(const QString &base_oid, const QString &sub_oid);

  /**
   * @brief Get the Snmp Message next object
   *
   * @param response
   * @param oid_array
   * @param oid_array_len
   * @param next_oid_array
   * @param next_oid_array_len
   * @param first_get
   * @param snmp_message_map_result
   * @return ACT_STATUS
   */
  static ACT_STATUS GetSnmpMessageNext(const netsnmp_pdu *response, oid *oid_array, size_t &oid_array_len,
                                       oid *next_oid_array, size_t next_oid_array_len, const bool &first_get,
                                       QMap<QString, QString> &snmp_message_map_result, bool &result_get_next_message);

  /**
   * @brief Get the SnmpMessage get object
   *
   * @param response
   * @param oid_array
   * @param oid_array_len
   * @param snmp_message_map_result
   * @return ACT_STATUS
   */
  static ACT_STATUS GetSnmpMessageGet(const netsnmp_pdu *response, oid *oid_array, size_t &oid_array_len,
                                      QMap<QString, QString> &snmp_message_map_result);

  /**
   * @brief Get the Snmp Message bulk object
   *
   * @param response
   * @param oid_array
   * @param oid_array_len
   * @param next_oid_array
   * @param next_oid_array_len
   * @param first_get
   * @param snmp_message_map_result
   * @param result_get_next_message
   * @return ACT_STATUS
   */
  static ACT_STATUS GetSnmpMessageBulk(const netsnmp_pdu *response, oid *oid_array, size_t &oid_array_len,
                                       oid *next_oid_array, size_t &next_oid_array_len, const bool &first_get,
                                       QMap<QString, QString> &snmp_message_map_result, bool &result_get_next_message);

 public:
  /**
   * @brief Get the Snmp subtree object
   *
   * @param oid
   * @param device
   * @param snmp_result
   */
  static ACT_STATUS GetSnmpSubTree(const QString &oid, const ActDevice &device,
                                   ActSnmpResult<ActSnmpMessageMap> &snmp_result);

  /**
   * @brief Get the Snmp Sub Tree By Bulk request object
   *
   * @param snmp_oid
   * @param device
   * @param snmp_result
   * @return ACT_STATUS
   */
  static ACT_STATUS GetSnmpSubTreeByBulk(const QString &snmp_oid, const ActDevice &device,
                                         ActSnmpResult<ActSnmpMessageMap> &snmp_result);

  // &snmpResult);

  /**
   * @brief Get the Snmp List object
   *
   * @param oid_list
   * @param device
   * @param snmp_result
   * @return ACT_STATUS
   */
  static ACT_STATUS GetSnmpList(const QList<QString> &oid_list, const ActDevice &device,
                                ActSnmpResult<ActSnmpMessageMap> &snmp_result);
};

#endif /* ACT_SNMPWALK_H */