/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_user.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace user {

/**
 * @brief Initial the user config database
 *
 * @return ACT_STATUS
 */
ACT_STATUS Init();

/**
 * @brief Retrieve data from user db and convert to UserSet format
 *
 * @param sys
 * @param last_assigned_user_id
 * @return ACT_STATUS
 */
ACT_STATUS RetrieveData(QSet<ActUser> &user_set, qint64 &last_assigned_user_id);

/**
 * @brief Write specific user to db
 *
 * @param user
 * @return ACT_STATUS
 */
ACT_STATUS WriteData(const ActUser &user);

/**
 * @brief Delete specify user from db
 *
 * @param id
 * @param username
 * @return ACT_STATUS
 */
ACT_STATUS DeleteUserFile(const qint64 &id, QString username);

}  // namespace user
}  // namespace database
}  // namespace act
