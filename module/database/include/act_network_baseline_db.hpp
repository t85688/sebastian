/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_network_baseline.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace networkbaseline {

/**
 * @brief Initial the NetworkBaseline config database
 *
 * @return ACT_STATUS
 */
ACT_STATUS Init();

/**
 * @brief Retrieve data from NetworkBaseline db and convert to DesignBaselineSet format
 *
 * @param mode
 * @param sys
 * @param last_assigned_baseline_id
 * @return ACT_STATUS
 */
ACT_STATUS RetrieveData(ActBaselineModeEnum mode, QSet<ActNetworkBaseline> &network_baseline_set,
                        qint64 &last_assigned_baseline_id);

/**
 * @brief Write specific NetworkBaseline to db
 *
 * @param mode
 * @param network_baseline
 * @return ACT_STATUS
 */
ACT_STATUS WriteData(ActBaselineModeEnum mode, const ActNetworkBaseline &network_baseline);

/**
 * @brief Delete specify NetworkBaseline from db
 *
 * @param mode
 * @param id
 * @param network_baseline_name
 * @return ACT_STATUS
 */
ACT_STATUS DeleteBaselineFile(ActBaselineModeEnum mode, const qint64 &id, QString network_baseline_name);

/**
 * @brief Update specify NetworkBaseline file name
 *
 * @param mode
 * @param id
 * @param old_name
 * @param new_name
 * @return ACT_STATUS
 */
ACT_STATUS UpdateNetworkBaselineFileName(ActBaselineModeEnum mode, const qint64 &id, QString old_name,
                                         QString new_name);

}  // namespace networkbaseline
}  // namespace database
}  // namespace act
