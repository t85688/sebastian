/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_system.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace system {

/**
 * @brief Initial the system config database
 *
 * @return ACT_STATUS
 */
ACT_STATUS Init();

/**
 * @brief Retrieve data from system db and convert to SystemConfig format
 *
 * @param sys
 * @return ACT_STATUS
 */
ACT_STATUS RetrieveData(ActSystem &sys);

}  // namespace system
}  // namespace database
}  // namespace act
