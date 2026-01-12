/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The ActAdminCycleTime class
 *
 * ActAdminCycleTime is a rational number of seconds, defined by an integer numerator and an integer denominator.
 * numerator / denominator
 */
class ActAdminCycleTime : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, numerator, Numerator);      ///< The numerator of Admin Cycle Time in seconds
  ACT_JSON_FIELD(quint32, denominator, Denominator);  ///< The denominator of Admin Cycle Time

 public:
  /**
   * @brief Construct a new Admin Cycle Time object
   *
   */
  ActAdminCycleTime() : numerator_(0), denominator_(1000000000) {}  // 1000 us

  /**
   * @brief Construct a new Admin Cycle Time object
   *
   * @param numerator
   * @param denominator
   */
  ActAdminCycleTime(const quint32 &numerator, const quint32 &denominator)
      : numerator_(numerator), denominator_(denominator) {}

  /**
   * @brief Returns floating-point microsecond value, rounded to three decimal places before returning
   *
   * @return microseconds
   */
  inline qreal ToMicroseconds() {
    if (denominator_ == 0) {
      return static_cast<qreal>(0);
    } else {
      qreal value = static_cast<qreal>(numerator_) * 1'000'000.0 / denominator_;
      // Result is rounded to three decimal places (999.999)
      return static_cast<qreal>(std::round(value * 1000.0) / 1000.0);
    }
  }
};
