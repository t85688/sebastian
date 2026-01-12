/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_status.hpp"
#include "act_system.hpp"
#include "json_utils.hpp"

/**
 * @brief Get the Item By Id in the specific QSet
 *
 * @tparam T
 * @param set
 * @param id
 * @param item
 * @return ACT_STATUS
 */
template <class T>
inline ACT_STATUS ActGetItemById(const QSet<T> set, const qint64 &id, T &item) {
  ACT_STATUS_INIT();

  // Iterator of the set
  typename QSet<T>::const_iterator iterator;
  iterator = set.find(T(id));

  // Check find result
  if (iterator == set.end()) {
    // Not found
    return std::make_shared<ActStatusNotFound>(QString::number(id));
  }

  item = *iterator;
  return act_status;
}

template <class T1>
inline ACT_STATUS ActGetListItemIndexById(const QList<T1> list, const qint64 &id, qint32 &index) {
  ACT_STATUS_INIT();
  index = -1;

  if (list.isEmpty()) {
    return std::make_shared<ActStatusNotFound>(QString::number(id));
  }

  for (qint32 i = 0; i < list.size(); i++) {
    if (list.at(i).GetId() == id) {
      // item = list.at(i);
      index = i;
      break;
    }
  }

  if (index == -1) {
    return std::make_shared<ActStatusNotFound>(QString("ID(%1)").arg(QString::number(id)));
  }

  return act_status;

  // for (auto each : list) {
  //   if (each.GetId() == id;) {
  //     break;
  //   }
  // }

  // // Iterator of the set
  // typename QList<T>::const_iterator iterator.const_be;
  // iterator = set.find(T(id));

  // // Check find result
  // if (iterator == set.end()) {
  //   // Not found
  //   return std::make_shared<ActStatusBase>(ActStatusType::kNotFound, ActSeverity::kCritical);
  // }

  // item = *iterator;
}

/**
 * @brief Read all JSON files within the same class type in the input direction.
 *
 * @param dir
 * @return QSet<T>
 */
// template <class T>
// static QSet<T> ReadAllSameClassFile(const QString &dir) {
//   QSet<T> class_files;
//   QVector<QJsonDocument> json_docs = ReadJsonFromDir(dir);

//   for (auto json_doc : json_docs) {
//     T class_file;
//     class_file.fromJson(json_doc.object());
//     class_files.insert(class_file);
//   }

//   return class_files;
// }

/**
 * @brief Transfer MAC address string to integer
 *
 * [feat:2495] Support fake MAC address
 *
 * @param mac_address
 * @param result
 * @return ACT_STATUS
 */
inline ACT_STATUS MacAddressToQInt64(const QString &mac_address, qint64 &result) {
  ACT_STATUS_INIT();

  result = 0;

  // Skip the transfer process if the input is empty
  if (mac_address.isEmpty()) {
    return act_status;
  }

  // Use QRegExp to match both "-" and ":" as separators
  QRegExp rx("[:\\-]");
  // Split the MAC address string into its components
  QStringList parts = mac_address.split(rx);
  if (parts.size() != 6) {
    QString error_msg = QString("Invalid MAC address format: %1").arg(mac_address);
    // qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Parse each component and combine them into a qint64
  for (int i = 0; i < 6; i++) {
    bool conversion_result = false;
    int byte_val = parts[i].toInt(&conversion_result, 16);
    if (!conversion_result || byte_val < 0 || byte_val > 255) {
      QString error_msg = QString("Invalid byte value: %1").arg(mac_address);
      qCritical() << __func__ << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    result = (result << 8) | byte_val;
  }

  return act_status;
}

/**
 * @brief Transfer MAC address integer to string
 *
 *
 * @param mac_int
 * @param result
 * @return ACT_STATUS
 */
inline ACT_STATUS QInt64ToMacAddress(const qint64 &mac_int, QString &result) {
  ACT_STATUS_INIT();

  result = "";

  // Skip the transfer process if the input is 0
  if (mac_int == 0) {
    return act_status;
  }

  // Convert the new MAC address integer to a formatted hexadecimal string (xx-xx-xx-xx-xx-xx)
  result = QString("%1").arg(mac_int, 12, 16, QChar('0'));
  result = result.toUpper();  // Ensure uppercase hexadecimal characters
  result = result.insert(2, '-').insert(5, '-').insert(8, '-').insert(11, '-').insert(14, '-');

  return act_status;
}

inline ACT_STATUS CheckMacAddress(const QString &mac_address) {
  ACT_STATUS_INIT();

  if (mac_address.size() == 0) {
    return act_status;
  }

  // XX-XX-XX-XX-XX-XX or XX:XX:XX:XX:XX:XX or 0:0:0:0:0:0
  QRegularExpression re("([0-9A-Fa-f]{1,2}[:-]){5}([0-9A-Fa-f]{1,2})|([0-9A-Fa-f]{1})$");

  // Check MAC address format
  QRegularExpressionMatch match = re.match(mac_address);
  if (!match.hasMatch()) {
    QString error_msg = QString("The MAC address(%1) is invalid").arg(mac_address);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  qint64 mac_int;
  act_status = MacAddressToQInt64(mac_address, mac_int);
  if (!IsActStatusSuccess(act_status)) {
    // qCritical() << __func__ << "Transfer MAC address" << mac_address << "failed";
    return act_status;
  }

  // Check not "00-00-00-00-00-00" or "FF-FF-FF-FF-FF-FF" MAC address
  if (mac_int == 0 || mac_address == ACT_BROADCAST_MAC_ADDRESS) {
    QString error_msg = QString("The MAC address(%1) is invalid").arg(mac_address);
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

inline ACT_STATUS TransferChassisIdToMacFormat(const QString &chassis_id, QString &result_mac) {
  ACT_STATUS_INIT();

  result_mac = "";

  // ChassisId To Mac
  // "00 90 E8 11 22 44" -> "00-90-E8-11-22-44"
  QString mac_address = chassis_id;
  mac_address = mac_address.replace(" ", "-").toUpper();

  // Check MAC address format
  act_status = CheckMacAddress(mac_address);
  if (!IsActStatusSuccess(act_status)) {
    // qWarning() << __func__ << "CheckMacAddress() failed.";
    return act_status;
  }

  // "00-90-E8-11-22-D" -> "00-90-E8-11-22-0D"
  qint64 mac_int = 0;
  MacAddressToQInt64(mac_address, mac_int);
  QInt64ToMacAddress(mac_int, mac_address);

  result_mac = mac_address;

  return act_status;
}