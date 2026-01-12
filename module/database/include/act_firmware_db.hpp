/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_firmware.hpp"
#include "act_json.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace firmware {

/**
 * @brief Initial the firmware config database
 *
 * @return ACT_STATUS
 */
ACT_STATUS Init();

/**
 * @brief Retrieve data from firmware db and convert to FirmwareSet format
 *
 * @param firmware_set
 * @param last_assigned_firmware_id
 * @return ACT_STATUS
 */
ACT_STATUS RetrieveData(QSet<ActFirmware> &firmware_set, qint64 &last_assigned_firmware_id);

/**
 * @brief Write specific firmware to db
 *
 * @param firmware
 * @return ACT_STATUS
 */
ACT_STATUS WriteData(const ActFirmware &firmware);

/**
 * @brief Delete specify firmware from db
 *
 * @param id
 * @param model_name
 * @param firmware_name
 * @return ACT_STATUS
 */
ACT_STATUS DeleteFirmwareFile(const qint64 &id, QString model_name, QString firmware_name);

}  // namespace firmware
}  // namespace database
}  // namespace act
