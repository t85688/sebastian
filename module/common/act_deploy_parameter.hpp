/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
/**
 * @brief The ACT Deploy parameter class
 *
 */
class ActDeployParameterBase : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, check_end_station, CheckEndStation);  ///< The CheckEndStation boolean

 public:
  /**
   * @brief Construct a new Act Deploy Parameter Firmware object
   *
   */
  ActDeployParameterBase() { this->check_end_station_ = true; }

  /**
   * @brief Construct a new Act Deploy Parameter Firmware object
   *
   * @param check_end_station
   */
  ActDeployParameterBase(const bool &check_end_station) : ActDeployParameterBase() {
    this->check_end_station_ = check_end_station;
  }
};

/**
 * @brief The ACT Deploy parameter class
 *
 */
class ActDeployParameterFirmware : public ActDeployParameterBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, firmware_name, FirmwareName);  ///< FirmwareName item

 public:
  /**
   * @brief Construct a new Act Deploy Parameter Firmware object
   *
   */
  ActDeployParameterFirmware() {
    this->firmware_name_ = "";
    this->SetCheckEndStation(true);
  }

  /**
   * @brief Construct a new Act Deploy Parameter Firmware object
   *
   * @param firmware_name
   */
  ActDeployParameterFirmware(const QString &firmware_name, const bool &check_end_station)
      : ActDeployParameterFirmware() {
    this->firmware_name_ = firmware_name;
    this->SetCheckEndStation(check_end_station);
  }
};
