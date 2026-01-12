#pragma once

#include <QDate>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QTime>

namespace LOGUTILS {
/**
 * @brief Initialize the ACT logging system
 * @return true/false
 */
bool InitACTLogging();

/**
 * @brief Set the log configuration
 * @param keep_days Number of days to keep logs
 */
void SetLogConfig(quint32 keep_days);

/**
 * @brief Delete old logs by day
 */
void DeleteOldLogsByDay();

}  // namespace LOGUTILS
