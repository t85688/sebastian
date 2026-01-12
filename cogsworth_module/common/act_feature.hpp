/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "act_system.hpp"
#include "simplecrypt.h"

class ActOperationFeature : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, reboot, Reboot);                          ///< Reboot item
  ACT_JSON_FIELD(bool, factory_default, FactoryDefault);         ///< FactoryDefault item
  ACT_JSON_FIELD(bool, firmware_upgrade, FirmwareUpgrade);       ///< FirmwareUpgrade item
  ACT_JSON_FIELD(bool, import_export, ImportExport);             ///< ImportExport item
  ACT_JSON_FIELD(bool, enable_snmp_service, EnableSNMPService);  ///< EnableSNMPService item
  ACT_JSON_FIELD(bool, event_log, EventLog);                     ///< EventLog  item
  ACT_JSON_FIELD(bool, locator, Locator);                        ///< Locator  item

  ACT_JSON_FIELD(bool, cli, CLI);  ///< CLI  item

  // ACT_JSON_FIELD(bool, import_export_configuration, ImportExportConfiguration);  ///< ImportExportConfiguration item

 public:
  QList<QString> key_order_;

  ActOperationFeature() {
    this->key_order_.append(QList<QString>({QString("Reboot"), QString("FactoryDefault"), QString("FirmwareUpgrade"),
                                            QString("ImportExport"), QString("EnableSNMPService"), QString("Locator"),
                                            QString("EventLog"), QString("CLI")}));

    this->reboot_ = false;
    this->factory_default_ = false;
    this->firmware_upgrade_ = false;
    this->import_export_ = false;
    this->enable_snmp_service_ = false;
    this->locator_ = false;
    this->event_log_ = false;
    this->cli_ = false;
  }
};

class ActTimeSettingItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, system_time, SystemTime);
  ACT_JSON_FIELD(bool, ptp, PTP);

 public:
  QList<QString> key_order_;

  ActTimeSettingItem() {
    this->key_order_.append(QList<QString>({QString("SystemTime"), QString("PTP")}));

    this->system_time_ = false;
    this->ptp_ = false;
  }
};

class ActVlanSettingItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, access_trunk_mode, AccessTrunkMode);
  ACT_JSON_FIELD(bool, hybrid_mode, HybridMode);
  ACT_JSON_FIELD(bool, management_vlan, ManagementVLAN);
  ACT_JSON_FIELD(bool, te_mstid, TEMSTID);
  ACT_JSON_FIELD(bool, default_pvid, DefaultPVID);
  ACT_JSON_FIELD(bool, default_pcp, DefaultPCP);
  ACT_JSON_FIELD(bool, per_stream_priority, PerStreamPriority);
  ACT_JSON_FIELD(bool, per_stream_priority_v2, PerStreamPriorityV2);

 public:
  QList<QString> key_order_;

  ActVlanSettingItem() {
    this->key_order_.append(QList<QString>(
        {QString("AccessTrunkMode"), QString("ManagementVLAN"), QString("HybridMode"), QString("TEMSTID"),
         QString("DefaultPVID"), QString("DefaultPCP"), QString("PerStreamPriority"), QString("PerStreamPriorityV2")}));

    this->access_trunk_mode_ = false;
    this->hybrid_mode_ = false;
    this->management_vlan_ = false;
    this->te_mstid_ = false;
    this->default_pvid_ = false;
    this->default_pcp_ = false;
    this->per_stream_priority_ = false;
    this->per_stream_priority_v2_ = false;
  }

  bool CheckSupportAnyOne() const {
    if (this->access_trunk_mode_ || this->hybrid_mode_ || this->management_vlan_ || this->te_mstid_ ||
        this->default_pvid_ || this->default_pcp_ || this->per_stream_priority_ || this->per_stream_priority_v2_) {
      return true;
    }
    return false;
  }
};

class ActStaticForwardSettingItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, unicast, Unicast);      ///< Unicast item
  ACT_JSON_FIELD(bool, multicast, Multicast);  ///< Multicast item

 public:
  QList<QString> key_order_;

  ActStaticForwardSettingItem() {
    this->key_order_.append(QList<QString>({QString("Unicast"), QString("Multicast")}));

    this->unicast_ = false;
    this->multicast_ = false;
  }

  bool CheckSupportAnyOne() const {
    if (this->unicast_ || this->multicast_) {
      return true;
    }
    return false;
  }
};

class ActSTPRSTPItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, rstp, RSTP);

  ACT_JSON_FIELD(bool, error_recovery_time, ErrorRecoveryTime);
  ACT_JSON_FIELD(bool, swift, Swift);
  ACT_JSON_FIELD(bool, port_rstp_enable, PortRSTPEnable);
  ACT_JSON_FIELD(bool, link_type, LinkType);
  ACT_JSON_FIELD(bool, bpdu_guard, BPDUGuard);
  ACT_JSON_FIELD(bool, root_guard, RootGuard);
  ACT_JSON_FIELD(bool, loop_guard, LoopGuard);
  ACT_JSON_FIELD(bool, bpdu_filter, BPDUFilter);

 public:
  QList<QString> key_order_;

  ActSTPRSTPItem() {
    this->key_order_.append(QList<QString>({QString("RSTP"), QString("ErrorRecoveryTime"), QString("Swift"),
                                            QString("PortRSTPEnable"), QString("LinkType"), QString("BPDUGuard"),
                                            QString("RootGuard"), QString("LoopGuard"), QString("BPDUFilter")}));

    this->rstp_ = false;
    this->error_recovery_time_ = false;
    this->swift_ = false;
    this->port_rstp_enable_ = false;
    this->link_type_ = false;
    this->bpdu_guard_ = false;
    this->root_guard_ = false;
    this->loop_guard_ = false;
    this->bpdu_filter_ = false;
  }
};

class ActPortSettingItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, admin_status, AdminStatus);  ///< AdminStatus item

 public:
  QList<QString> key_order_;

  ActPortSettingItem() {
    this->key_order_.append(QList<QString>({QString("AdminStatus")}));

    this->admin_status_ = false;
  }
};

class ActTSNItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, ieee802dot1cb, IEEE802Dot1CB);    ///< IEEE802Dot1CB item
  ACT_JSON_FIELD(bool, ieee802dot1qbv, IEEE802Dot1Qbv);  ///< IEEE802Dot1Qbv item

 public:
  QList<QString> key_order_;

  ActTSNItem() {
    this->key_order_.append(QList<QString>({QString("IEEE802Dot1CB"), QString("IEEE802Dot1Qbv")}));

    this->ieee802dot1cb_ = false;
    this->ieee802dot1qbv_ = false;
  }

  bool CheckSupportAnyOne() const {
    if (this->ieee802dot1cb_ || this->ieee802dot1qbv_) {
      return true;
    }
    return false;
  }
};

class ActTimeSyncSettingItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, ieee1588_2008, IEEE1588_2008);
  ACT_JSON_FIELD(bool, ieee802dot1as_2011, IEEE802Dot1AS_2011);
  ACT_JSON_FIELD(bool, iec61850_2016, IEC61850_2016);
  ACT_JSON_FIELD(bool, ieeec37dot238_2017, IEEEC37Dot238_2017);
  ACT_JSON_FIELD(bool, ieee1588_2008_clock_type, IEEE1588_2008_ClockType);
  ACT_JSON_FIELD(bool, ieee1588_2008_clock_mode, IEEE1588_2008_ClockMode);
  ACT_JSON_FIELD(bool, ieee1588_2008_maximum_steps_removed, IEEE1588_2008_MaximumStepsRemoved);

 public:
  QList<QString> key_order_;

  ActTimeSyncSettingItem() {
    this->key_order_.append(
        QList<QString>({QString("IEEE1588_2008"), QString("IEEE802Dot1AS_2011"), QString("IEC61850_2016"),
                        QString("IEEEC37Dot238_2017"), QString("IEEE1588_2008_ClockType"),
                        QString("IEEE1588_2008_ClockMode"), QString("IEEE1588_2008_MaximumStepsRemoved")}));

    this->ieee1588_2008_ = false;
    this->ieee802dot1as_2011_ = false;
    this->iec61850_2016_ = false;
    this->ieeec37dot238_2017_ = false;
    this->ieee1588_2008_clock_type_ = false;
    this->ieee1588_2008_clock_mode_ = false;
    this->ieee1588_2008_maximum_steps_removed_ = false;
  }

  bool CheckSupportAnyOne() const {
    if (this->ieee1588_2008_ || this->ieee802dot1as_2011_ || this->iec61850_2016_ || this->ieeec37dot238_2017_) {
      return true;
    }
    return false;
  }
};

class ActConfigurationFeature : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, network_setting, NetworkSetting);
  ACT_JSON_FIELD(bool, user_account, UserAccount);
  ACT_JSON_FIELD(bool, information_setting, InformationSetting);
  ACT_JSON_FIELD(bool, management_interface, ManagementInterface);
  ACT_JSON_FIELD(bool, login_policy, LoginPolicy);
  ACT_JSON_FIELD(bool, snmp_trap_setting, SNMPTrapSetting);
  ACT_JSON_FIELD(bool, syslog_setting, SyslogSetting);
  ACT_JSON_FIELD(bool, loop_protection, LoopProtection);
  ACT_JSON_FIELD(bool, linkup_delay, LinkupDelay);
  ACT_JSON_FIELD(bool, prp_hsr, PRPHSR);
  ACT_JSON_FIELD(bool, supervision_frame, SupervisionFrame);
  ACT_JSON_OBJECT(ActTimeSettingItem, time_setting, TimeSetting);
  ACT_JSON_OBJECT(ActVlanSettingItem, vlan_setting, VLANSetting);
  ACT_JSON_OBJECT(ActStaticForwardSettingItem, static_forward_setting, StaticForwardSetting);
  ACT_JSON_OBJECT(ActSTPRSTPItem, stp_rstp, STPRSTP);
  ACT_JSON_OBJECT(ActPortSettingItem, port_setting, PortSetting);
  ACT_JSON_OBJECT(ActTimeSyncSettingItem, time_sync_setting, TimeSyncSetting);
  ACT_JSON_OBJECT(ActTSNItem, tsn, TSN);
  ACT_JSON_FIELD(bool, check_config_synchronization, CheckConfigSynchronization);
  // item

 public:
  QList<QString> key_order_;

  ActConfigurationFeature() {
    this->key_order_.append(QList<QString>(
        {QString("NetworkSetting"), QString("UserAccount"), QString("InformationSetting"),
         QString("ManagementInterface"), QString("LoginPolicy"), QString("SNMPTrapSetting"), QString("SyslogSetting"),
         QString("LoopProtection"), QString("LinkupDelay"), QString("PRPHSR"), QString("SupervisionFrame"),
         QString("TimeSetting"), QString("VLANSetting"), QString("StaticForwardSetting"), QString("STPRSTP"),
         QString("PortSetting"), QString("TimeSyncSetting"), QString("TSN"), QString("CheckConfigSynchronization")}));

    this->network_setting_ = false;
    this->user_account_ = false;
    this->information_setting_ = false;
    this->management_interface_ = false;
    this->login_policy_ = false;
    this->snmp_trap_setting_ = false;
    this->syslog_setting_ = false;
    this->loop_protection_ = false;
    this->linkup_delay_ = false;
    this->prp_hsr_ = false;
    this->supervision_frame_ = false;
    this->check_config_synchronization_ = false;
  }

  bool CheckSupportAnyOne() const {
    if (this->network_setting_ || this->user_account_ || this->information_setting_ || this->management_interface_ ||
        this->login_policy_ || this->snmp_trap_setting_ || this->syslog_setting_ || this->loop_protection_ ||
        this->linkup_delay_ || this->prp_hsr_ || this->supervision_frame_) {
      return true;
    }

    if (this->time_setting_.GetSystemTime()) {
      return true;
    }

    if (this->vlan_setting_.CheckSupportAnyOne()) {
      return true;
    }

    if (this->static_forward_setting_.CheckSupportAnyOne()) {
      return true;
    }

    if (this->stp_rstp_.GetRSTP()) {
      return true;
    }

    if (this->port_setting_.GetAdminStatus()) {
      return true;
    }

    if (this->time_sync_setting_.CheckSupportAnyOne()) {
      return true;
    }

    if (this->tsn_.CheckSupportAnyOne()) {
      return true;
    }

    return false;
  }
};

class ActMonitorBasicStatusItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, system_utilization, SystemUtilization);
  ACT_JSON_FIELD(bool, port_status, PortStatus);
  ACT_JSON_FIELD(bool, fiber_check, FiberCheck);

 public:
  QList<QString> key_order_;

  ActMonitorBasicStatusItem() {
    this->key_order_.append(
        QList<QString>({QString("SystemUtilization"), QString("PortStatus"), QString("FiberCheck")}));

    this->system_utilization_ = false;
    this->port_status_ = false;
    this->fiber_check_ = false;
  }
};

class ActMonitorTrafficItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, tx_total_octets, TxTotalOctets);           ///< TxTotalOctets item
  ACT_JSON_FIELD(bool, tx_total_packets, TxTotalPackets);         ///< TxTotalPackets item
  ACT_JSON_FIELD(bool, traffic_utilization, TrafficUtilization);  ///< TrafficUtilization item

 public:
  QList<QString> key_order_;

  ActMonitorTrafficItem() {
    this->key_order_.append(
        QList<QString>({QString("TxTotalOctets"), QString("TxTotalPackets"), QString("TrafficUtilization")}));

    this->tx_total_octets_ = false;
    this->tx_total_packets_ = false;
    this->traffic_utilization_ = false;
  }
};

class ActMonitorRedundancyItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, rstp, RSTP);  ///< RSTP item

 public:
  QList<QString> key_order_;

  ActMonitorRedundancyItem() {
    this->key_order_.append(QList<QString>({QString("RSTP")}));

    this->rstp_ = false;
  }
};

class ActMonitorTimeSynchronizationItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, ieee1588_2008, IEEE1588_2008);            ///< IEEE1588_2008 item
  ACT_JSON_FIELD(bool, ieee802dot1as_2011, IEEE802Dot1AS_2011);  ///< IEEE802Dot1AS_2011 item

 public:
  QList<QString> key_order_;

  ActMonitorTimeSynchronizationItem() {
    this->key_order_.append(QList<QString>({QString("IEEE1588_2008"), QString("IEEE802Dot1AS_2011")}));

    this->ieee1588_2008_ = false;
    this->ieee802dot1as_2011_ = false;
  }
};

class ActMonitorFeature : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActMonitorBasicStatusItem, basic_status, BasicStatus);  ///< BasicStatus item
  ACT_JSON_OBJECT(ActMonitorTrafficItem, traffic, Traffic);               ///< Traffic item
  ACT_JSON_OBJECT(ActMonitorRedundancyItem, redundancy, Redundancy);      ///< Redundancy item
  ACT_JSON_OBJECT(ActMonitorTimeSynchronizationItem, time_synchronization,
                  TimeSynchronization);  ///< TimeSynchronization item

 public:
  QList<QString> key_order_;

  ActMonitorFeature() {
    this->key_order_.append(QList<QString>(
        {QString("BasicStatus"), QString("Traffic"), QString("Redundancy"), QString("TimeSynchronization")}));
  }
};

class ActIdentifyItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, model_name, ModelName);              ///< ModelName item
  ACT_JSON_FIELD(bool, vendor_id, VendorID);                ///< VendorID item
  ACT_JSON_FIELD(bool, firmware_version, FirmwareVersion);  ///< FirmwareVersion item

 public:
  QList<QString> key_order_;

  ActIdentifyItem() {
    this->key_order_.append(QList<QString>({QString("ModelName"), QString("VendorID"), QString("FirmwareVersion")}));

    this->model_name_ = false;
    this->vendor_id_ = false;
    this->firmware_version_ = false;
  }
};

class ActDeviceInformationItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, device_name, DeviceName);
  ACT_JSON_FIELD(bool, ip_configuration, IPConfiguration);
  ACT_JSON_FIELD(bool, location, Location);
  ACT_JSON_FIELD(bool, product_revision, ProductRevision);
  ACT_JSON_FIELD(bool, system_uptime, SystemUptime);
  ACT_JSON_FIELD(bool, redundant_protocol, RedundantProtocol);
  ACT_JSON_FIELD(bool, serial_number, SerialNumber);
  ACT_JSON_FIELD(bool, modular_info, ModularInfo);
  ACT_JSON_FIELD(bool, port_info, PortInfo);

  ACT_JSON_FIELD(bool, mac_table, MACTable);
  ACT_JSON_FIELD(bool, interface_name, InterfaceName);
  ACT_JSON_FIELD(bool, interface_mac, InterfaceMAC);
  ACT_JSON_FIELD(bool, port_speed, PortSpeed);

 public:
  QList<QString> key_order_;

  ActDeviceInformationItem() {
    this->key_order_.append(
        QList<QString>({QString("DeviceName"), QString("IPConfiguration"), QString("Location"),
                        QString("ProductRevision"), QString("SystemUptime"), QString("RedundantProtocol"),
                        QString("SerialNumber"), QString("ModularInfo"), QString("PortInfo"), QString("MACTable"),
                        QString("InterfaceName"), QString("InterfaceMAC"), QString("PortSpeed")}));

    this->device_name_ = false;
    this->ip_configuration_ = false;
    this->location_ = false;
    this->product_revision_ = false;
    this->system_uptime_ = false;
    this->redundant_protocol_ = false;
    this->serial_number_ = false;
    this->modular_info_ = false;
    this->port_info_ = false;
    this->mac_table_ = false;
    this->interface_name_ = false;
    this->interface_mac_ = false;
    this->port_speed_ = false;
  }
};

class ActAutoScanFeature : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, broadcast_search, BroadcastSearch);                           ///< BroadcastSearch item
  ACT_JSON_OBJECT(ActIdentifyItem, identify, Identify);                              ///< Identify item
  ACT_JSON_FIELD(bool, lldp, LLDP);                                                  ///< LLDP item
  ACT_JSON_OBJECT(ActDeviceInformationItem, device_information, DeviceInformation);  ///< DeviceInformation item

 public:
  QList<QString> key_order_;

  ActAutoScanFeature() {
    this->key_order_.append(QList<QString>(
        {QString("BroadcastSearch"), QString("Identify"), QString("LLDP"), QString("DeviceInformation")}));

    this->broadcast_search_ = false;
    this->lldp_ = false;
  }
};

/**
 * @brief The ACT Device feature group class
 *
 */
class ActFeatureGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActAutoScanFeature, auto_scan, AutoScan);                ///< AutoScan item
  ACT_JSON_OBJECT(ActOperationFeature, operation, Operation);              ///< Operation item
  ACT_JSON_OBJECT(ActConfigurationFeature, configuration, Configuration);  ///< Configuration item
  ACT_JSON_OBJECT(ActMonitorFeature, monitor, Monitor);                    ///< Monitor item

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Feature Group object
   *
   */
  ActFeatureGroup() {
    this->key_order_.append(
        QList<QString>({QString("AutoScan"), QString("Operation"), QString("Configuration"), QString("Monitor")}));
  }
};
