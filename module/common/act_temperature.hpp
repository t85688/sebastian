/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Act Operating Temperature in Celsius class
 *
 */
class ActOperatingTemperatureC : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint16, min, Min);
  ACT_JSON_FIELD(qint16, max, Max);

 public:
  ActOperatingTemperatureC() {
    this->min_ = -10;
    this->max_ = 60;
  }
};
