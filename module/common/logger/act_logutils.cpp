#include "act_logutils.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QMutex>
#include <QStorageInfo>
#include <QTime>
#include <iostream>

#include "act_system.hpp"

namespace LOGUTILS {
static QString logFileName;
static QString currentLogDay;
static quint32 current_log_keep_days;
static quint64 current_min_free_bytes;

static QString getAppName() { return GetActAppName(); }

static QString makeLogFileName() {
  QDateTime now = QDateTime::currentDateTime();
  return QString("%1/%2.txt").arg(GetLogPath()).arg(getAppName() + now.toString("_yyyy_MM_dd_HH_mm_ss"));
}

void InitLogFileName() {
  logFileName = makeLogFileName();
  currentLogDay = QDate::currentDate().toString("yyyy_MM_dd");
}

static qint64 getDirSize(const QString &path) {
  qint64 size = 0;
  QDir dir(path);
  QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QFileInfo &fi : list) {
    if (fi.isDir())
      size += getDirSize(fi.absoluteFilePath());
    else
      size += fi.size();
  }
  return size;
}

static bool hasEnoughSpace(const QString &path, quint64 minBytes) {
  QStorageInfo storage(path);
  return storage.isValid() && storage.bytesAvailable() > minBytes;
}

void SetLogConfig(quint32 keep_days) { current_log_keep_days = keep_days; }

void DeleteOldLogsByDay() {
  quint32 minDays = current_log_keep_days;
  QDir dir(GetLogPath());
  dir.setFilter(QDir::Files | QDir::NoSymLinks);
  QFileInfoList files = dir.entryInfoList();
  QDate today = QDate::currentDate();

  for (const QFileInfo &fi : files) {
    // Assumes filename format is AppName_yyyy_MM_dd_HH_mm_ss.txt
    QString dayStr = fi.baseName().section('_', 1, 3, QString::SectionSkipEmpty);
    QDate fileDate = QDate::fromString(dayStr, "yyyy_MM_dd");
    if (fileDate.isValid()) {
      int daysDiff = fileDate.daysTo(today);
      if (daysDiff >= static_cast<int>(minDays)) {
        QFile::remove(fi.absoluteFilePath());
        continue;
      }
    } else {
      // If date is invalid, treat as old and remove
      QFile::remove(fi.absoluteFilePath());
    }
  }
}

void ActMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  // Check if day changed, rotate log file
  QString today = QDate::currentDate().toString("yyyy_MM_dd");
  if (today != currentLogDay) {
    // Rotate by day, and delete old logs by keep days
    InitLogFileName();
    DeleteOldLogsByDay();
  }

  // Check if log file exists, if not, create it and do rotate
  QFile outFileCheck(logFileName);
  if (!outFileCheck.exists()) {
    InitLogFileName();
    DeleteOldLogsByDay();
  } else if (outFileCheck.size() >= ACT_LOG_SIZE) {
    // Check file size and if needed create new log!
    InitLogFileName();
  }

  static QMutex mutex;
  QMutexLocker lock(&mutex);

  QString text;
  switch (type) {
    case QtFatalMsg:
      text = QString("FATAL");  // Changed from [Fatal]
      break;
    case QtCriticalMsg:
      text = QString("ERRO");  // Changed from [Critical]
      break;
    case QtWarningMsg:
      text = QString("WARN");  // Changed from [Warning]
      break;
    case QtInfoMsg:
      text = QString("INFO");  // Changed from [Info]
      break;
    case QtDebugMsg:
      text = QString("DBUG");  // Changed from [Debug]
      break;
  }
  QLocale locale(QLocale("en_US"));
  QDateTime date = QDateTime::currentDateTime();
  QString current_date = locale.toString(date, "yyyy-MM-dd hh:mm:ss.zzz");  // Changed format

  // New log format
  QString message = QString("[%1] %2 ").arg(text).arg(current_date);

  // message body - append directly
  message.append(QString("%1 ").arg(msg));

  // New context_info formatting
  QString filePathStr = QString(context.file);
  QFileInfo fileInfo(filePathStr);
  QString fileName = fileInfo.fileName();
  QString parentDirName = fileInfo.dir().dirName();
  QString lineNumberString;

  lineNumberString = QString::number(context.line);

  // if (fileName.endsWith(".cpp", Qt::CaseInsensitive) || fileName.endsWith(".hpp", Qt::CaseInsensitive) ||
  //     fileName.endsWith(".h", Qt::CaseInsensitive)) {
  //   // No change needed for line number justification based on new format
  //   lineNumberString = QString::number(context.line);
  // } else {
  //   lineNumberString = QString::number(context.line);
  // }

  // Construct the code field (debug builds only)
#ifdef _DEBUG
  QString formatted_context_info = QString("code=./%1/%2:%3").arg(parentDirName).arg(fileName).arg(lineNumberString);
  message.append(formatted_context_info);
#endif

  std::cout << message.toStdString().c_str() << "\\n";
  QFile outFile(logFileName);
  outFile.open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream ts(&outFile);
  ts << message << Qt::endl;
}

bool InitACTLogging() {
  QString logPath = GetLogPath();
  if (!QDir(logPath).exists()) {
    QDir().mkpath(logPath);
  }

  // Only initialize log file name and install message handler
  InitLogFileName();
  QFile outFile(logFileName);
  if (outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    qInstallMessageHandler(ActMessageHandler);
    return true;
  } else {
    return false;
  }
}

}  // namespace LOGUTILS
