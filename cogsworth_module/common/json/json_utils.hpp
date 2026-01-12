/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>

/**
 * @brief Read json file from system
 *
 * @return QJsonDocument
 */
// static QJsonDocument ReadJsonFile(const QString& file_path) {
//   QFile file(file_path);

//   // Open the json file
//   if (!file.open(QIODevice::ReadWrite)) {
//     qCritical() << "Open json file failed:" << file_path;
//   }

//   // Read all contents at one time
//   QByteArray ba = file.readAll();

//   // Close the json file
//   file.close();
//   return QJsonDocument::fromJson(ba);
// }

/**
 * @brief Read json file from system
 *
 * @return QJsonDocument
 */
// static QVector<QJsonDocument> ReadJsonFromDir(const QString& dir) {
//   QVector<QJsonDocument> json_documents;

//   QDirIterator iterator(dir, QDirIterator::Subdirectories);
//   while (iterator.hasNext()) {
//     QFile file(iterator.next());
//     // Open the json file
//     if (!file.open(QIODevice::ReadWrite)) {
//       continue;
//     }
//     qDebug() << "Open File:" << file.fileName();
//     // Read all contents at one time
//     QByteArray ba = file.readAll();

//     // Close the license file
//     file.close();
//     json_documents.append(QJsonDocument::fromJson(ba));
//   }
//   return json_documents;
// }
