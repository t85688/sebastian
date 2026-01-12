/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_topology.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace topology {

/**
 * @brief Initial the topology config database
 *
 * @return ACT_STATUS
 */
ACT_STATUS Init();

/**
 * @brief Retrieve data from topology db and convert to TopologySet format
 *
 * @param topology_set
 * @param last_assigned_topology_id
 * @return ACT_STATUS
 */
ACT_STATUS RetrieveData(QSet<ActTopology> &topology_set, qint64 &last_assigned_topology_id);

/**
 * @brief Write specific topology to db
 *
 * @param topology
 * @return ACT_STATUS
 */
ACT_STATUS WriteData(const ActTopology &topology);

/**
 * @brief Delete specify topology from db
 *
 * @param id
 * @param topology_name
 * @return ACT_STATUS
 */
ACT_STATUS DeleteTopologyFile(const qint64 &id, QString topology_name);

/**
 * @brief Save the topology icon to database
 *
 * @param id The id of the related topology
 * @param image
 * @return ACT_STATUS
 */
ACT_STATUS SaveTopologyIcon(const qint64 &id, const QString &url_base64_data);

}  // namespace topology
}  // namespace database
}  // namespace act
