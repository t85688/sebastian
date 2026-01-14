#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>

#include "act_core.hpp"
#include "quazip.h"
#include "quazipfile.h"

namespace act {
namespace core {

// Recursive to get all files in the folder
void GetAllFiles(const QString &folderPath, QStringList &fileList, const QString &basePath) {
  QDir dir(folderPath);
  QFileInfoList fileInfos = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QFileInfo &fileInfo : fileInfos) {
    if (fileInfo.isDir()) {
      // child forder
      GetAllFiles(fileInfo.filePath(), fileList, basePath);
    } else {
      // Add relative path to file list
      QString relativePath = fileInfo.filePath();
      relativePath = relativePath.mid(basePath.length() + 1);  // remove base path
      fileList.append(relativePath);
    }
  }
}

ACT_STATUS ActCore::CompressFolder(const QString &folderPath, const QString &zipFilePath) {
  ACT_STATUS_INIT();

  // Get all files
  QStringList fileList;
  GetAllFiles(folderPath, fileList, folderPath);

  QuaZip zip(zipFilePath);
  if (!zip.open(QuaZip::mdCreate)) {
    QString error_msg = QString("Cannot create compress file: %1").arg(zipFilePath);
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QuaZipFile outFile(&zip);
  for (const QString &relativeFilePath : fileList) {
    QString absoluteFilePath = folderPath + "/" + relativeFilePath;

    QFile inputFile(absoluteFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
      QString error_msg = QString("Cannot read file: %1").arg(absoluteFilePath);
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // open QuaZip file
    QuaZipNewInfo fileInfo(relativeFilePath, absoluteFilePath);
    if (!outFile.open(QIODevice::WriteOnly, fileInfo)) {
      QString error_msg = QString("Cannot write to zip file: %1").arg(absoluteFilePath);
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // write file data
    outFile.write(inputFile.readAll());
    outFile.close();
    inputFile.close();

    if (outFile.getZipError() != UNZ_OK) {
      QString error_msg = QString("Write to zip file error: %1").arg(absoluteFilePath);
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  zip.close();
  if (zip.getZipError() != 0) {
    QString error_msg = QString("Close zip file error: %1").arg(zipFilePath);
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return ACT_STATUS_SUCCESS;
}

// 讀取 ZIP 壓縮檔並解壓到目標目錄
// read file
ACT_STATUS ActCore::ReadFileContent(const QString &filePath, QByteArray &fileContent) {
  ACT_STATUS_INIT();

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    QString error_msg = QString("Open device config file failed: %1").arg(file.fileName());
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // QByteArray byteArray;
  constexpr qint64 bufferSize = 1024 * 1024;  // 1 MB 緩衝區
  QByteArray buffer;
  while (!file.atEnd()) {
    buffer = file.read(bufferSize);
    if (buffer.isEmpty()) {
      file.close();
      QString error_msg = QString("Read file error: %1").arg(file.fileName());
      return std::make_shared<ActBadRequest>(error_msg);
    }
    fileContent.append(buffer);
  }

  file.close();

  return ACT_STATUS_SUCCESS;
}

ACT_STATUS ActCore::UnZipFile(const QString &zip_file_path, const QString &destination_path) {
  ACT_STATUS_INIT();

  if (!QFile::exists(zip_file_path)) {
    QString error_msg = QString("File does not exist: %1").arg(zip_file_path);
    return std::make_shared<ActBadRequest>(error_msg);
  }

  QuaZip zip(zip_file_path);
  if (!zip.open(QuaZip::mdUnzip)) {
    QString error_msg = QString("Open zip file failed: %1").arg(zip_file_path);
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check destination_path exists
  QDir fw_file_dir(destination_path);
  if (!fw_file_dir.exists()) {
    if (!QDir().mkpath(destination_path)) {
      qDebug() << __func__ << "mkpath() failed:" << destination_path;
      return std::make_shared<ActStatusInternalError>("CreateFolderFailed");
    }
  }

  // // Iterate through each file in the ZIP archive
  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
    QString file_name = zip.getCurrentFileName();
    // qDebug() << "FileName:" << file_name;

    // Open the current file in the ZIP archive
    QuaZipFile zip_file(&zip);
    if (!zip_file.open(QIODevice::ReadOnly)) {
      qWarning() << "Can't read file in ZIP:" << file_name;
      continue;
    }

    // Handle file paths, including nested directory structures
    QString output_file_path = destination_path + "/" + file_name;
    QFile out_file(output_file_path);

    // If it's a directory, create the corresponding directory
    QFileInfo file_info(output_file_path);
    if (file_info.isDir() || file_name.endsWith("/")) {
      QDir().mkpath(file_info.absoluteFilePath());
      zip_file.close();
      continue;
    }

    // Ensure the file's parent directory exists
    QDir().mkpath(file_info.absolutePath());

    // Create file
    if (!out_file.open(QIODevice::WriteOnly)) {
      qWarning() << "Can't not create file:" << output_file_path;
      zip_file.close();
      continue;
    }

    out_file.write(zip_file.readAll());
    out_file.close();
    zip_file.close();
  }

  zip.close();

  return ACT_STATUS_SUCCESS;
}

}  // namespace core
}  // namespace act