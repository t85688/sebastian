/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_project.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace project {

/**
 * @brief Initial the project config database
 *
 * @param project_db_exist
 * @return ACT_STATUS
 */
ACT_STATUS Init(bool &project_db_exist);

/**
 * @brief Retrieve data from project db and convert to ProjectSet format
 *
 * @param project_set
 * @param last_assigned_project_id
 * @return ACT_STATUS
 */
ACT_STATUS RetrieveData(QSet<ActProject> &project_set, qint64 &last_assigned_project_id);

/**
 * @brief Write specify project to db
 *
 * @param project
 * @return ACT_STATUS
 */
ACT_STATUS WriteData(const ActProject &project);

/**
 * @brief Delete specify project from db
 *
 * @param id
 * @param project_name
 * @return ACT_STATUS
 */
ACT_STATUS DeleteProjectFile(const qint64 &id, QString project_name);

/**
 * @brief Update specify project file name
 *
 * @param id
 * @param old_project_name
 * @param new_project_name
 * @return ACT_STATUS
 */
ACT_STATUS UpdateProjectFileName(const qint64 &id, QString old_project_name, QString new_project_name);

}  // namespace project
}  // namespace database
}  // namespace act
