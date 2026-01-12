/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "topology/act_device.hpp"

/**
 * @brief The ActScanIpRangeEntry class
 *
 */
class ActScanIpRangeEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, start_ip, StartIp);  ///< The start IP of the ip assignment
  ACT_JSON_FIELD(QString, end_ip, EndIp);

  ACT_JSON_OBJECT(ActDeviceAccount, account, Account);                           ///< Device connection account item
  ACT_JSON_OBJECT(ActSnmpConfiguration, snmp_configuration, SnmpConfiguration);  ///< SNMP configuration item
  ACT_JSON_OBJECT(ActNetconfConfiguration, netconf_configuration,
                  NetconfConfiguration);  ///< Netconf configuration item
  ACT_JSON_OBJECT(ActRestfulConfiguration, restful_configuration,
                  RestfulConfiguration);  ///< Restful configuration item

  ACT_JSON_FIELD(bool, enable_snmp_setting, EnableSnmpSetting);  ///< The Enable SnmpSetting item
  ACT_JSON_FIELD(bool, auto_probe, AutoProbe);                   ///< The AutoProbe item

 private:
 public:
  /*
   * @brief [feat:1662] Remove the password related fields in the project
   *
   * @return ACT_STATUS
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    // Hide password in the project setting
    this->account_.HidePassword();
    this->snmp_configuration_.HidePassword();

    return act_status;
  }

  /**
   * @brief Encrypt the password field
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    this->account_.EncryptPassword();
    this->snmp_configuration_.EncryptPassword();

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    // Decrypt password in the project setting
    this->account_.DecryptPassword();
    this->snmp_configuration_.DecryptPassword();

    return act_status;
  }

  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   */
  ActScanIpRangeEntry() {
    id_ = -1;
    start_ip_ = "";
    end_ip_ = "";
    enable_snmp_setting_ = true;
    auto_probe_ = true;
  }

  ActScanIpRangeEntry(const qint64 &id) : ActScanIpRangeEntry() { this->id_ = id; }

  /**
   * @brief Construct a new Act Scan Range Entry object
   *
   * @param id
   * @param name
   */
  ActScanIpRangeEntry(const qint64 &id, const QString &start_ip, const QString &end_ip, const ActDeviceAccount &account,
                      const ActSnmpConfiguration &snmp_configuration,
                      const ActNetconfConfiguration &netconf_configuration,
                      const ActRestfulConfiguration &restful_configuration, const bool &enable_snmp_setting,
                      const bool &auto_probe)
      : ActScanIpRangeEntry() {
    id_ = id;
    start_ip_ = start_ip;
    end_ip_ = end_ip;
    account_ = account;
    snmp_configuration_ = snmp_configuration;
    netconf_configuration_ = netconf_configuration;
    restful_configuration_ = restful_configuration;
    enable_snmp_setting_ = enable_snmp_setting;
    auto_probe_ = auto_probe;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActScanIpRangeEntry &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActScanIpRangeEntry &x, const ActScanIpRangeEntry &y) { return x.id_ == y.id_; }
};

/**
 * @brief The ActScanIpRange class
 *
 */
class ActScanIpRange : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActScanIpRangeEntry, scan_ip_range_entries, ScanIpRangeEntries);
};
