/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

class ActLLDPData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Local
  ACT_JSON_FIELD(QString, loc_chassis_id, LocChassisId);                   ///< LocChassisId item
  ACT_JSON_FIELD(QString, loc_sys_name, LocSysName);                       ///< LocSysName item
  ACT_JSON_FIELD(QString, loc_sys_desc, LocSysDesc);                       ///< LocSysDesc item
  ACT_JSON_QT_DICT(QMap, qint64, QString, loc_port_id_map, LocPortIdMap);  ///< The LocPortId map <PortID, LocPortID>

  // Remote
  ACT_JSON_QT_DICT(QMap, qint64, qint64, rem_port_chassis_id_subtype_map,
                   RemPortChassisIdSubtypeMap);  ///< The RemPortChassisIdSubtype map <PortID, RemChassisIDSubtype>
  ACT_JSON_QT_DICT(QMap, qint64, QString, rem_port_chassis_id_map,
                   RemPortChassisIdMap);  ///< The RemPortChassisId map <PortID, RemChassisID>
  ACT_JSON_QT_DICT(QMap, qint64, qint64, rem_port_id_subtype_map,
                   RemPortIdSubtypeMap);  ///< The RemPortIdSubtype map <PortID, RemPortIDSubType>
  ACT_JSON_QT_DICT(QMap, qint64, QString, rem_port_id_map, RemPortIdMap);  ///< The RemPortId map <PortID, RemPortID>

 public:
  ActLLDPData() {}

  ActLLDPData(const QMap<qint64, qint64> &rem_port_chassis_id_subtype_map,
              const QMap<qint64, QString> &rem_port_chassis_id_map, const QMap<qint64, qint64> &rem_port_id_subtype_map,
              const QMap<qint64, QString> &rem_port_id_map)
      : ActLLDPData() {
    this->rem_port_chassis_id_subtype_map_ = rem_port_chassis_id_subtype_map;
    this->rem_port_chassis_id_map_ = rem_port_chassis_id_map;
    this->rem_port_id_subtype_map_ = rem_port_id_subtype_map;
    this->rem_port_id_map_ = rem_port_id_map;
  }
};
