/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The GCL's ActAdminBaseTime class
 *
 */
class ActAdminBaseTime : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, seconds, Second);                        ///< The seconds of Admin Base Time
  ACT_JSON_FIELD(quint64, fractional_seconds, FractionalSeconds);  ///< The fractional seconds of Admin Base Time

 public:
  const static inline QString kStrScanfFormat = "%4d:%2d:%2d/%2d:%2d:%2d:%f";
  const static inline QString kStrRegex = "^\\d{4}:\\d{2}:\\d{2}/\\d{2}:\\d{2}:\\d{2}:[+-]?(\\d{0,6}\\.)?\\d+";
  const static inline QString kStrFormatForRead = "YYYY:MM:DD/HH:MM:SS:MS(With float)";

  /**
   * @brief Construct a new Admin Base Time object
   *
   */
  ActAdminBaseTime() : seconds_(0), fractional_seconds_(0) {}

  /**
   * @brief Construct a new Admin Base Time object
   *
   * @param seconds
   * @param fractional_seconds
   */
  ActAdminBaseTime(const quint64 &seconds, const quint64 &fractional_seconds)
      : seconds_(seconds), fractional_seconds_(fractional_seconds) {}
};
