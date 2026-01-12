#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::UpdateSystem(ActSystem &system) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check system configuration
  this->SetSystemConfig(system);

  system.SetDataVersion(ACT_SYSTEM_DATA_VERSION);

  // Write the change to system configuration
  // kene+
  /*
  QString db_name(ACT_SYSTEM_DB_NAME);
  */
  QString db_name(act::database::GetSystemDbName());
  // kene-
  act_status = act::database::WriteToDB(system, db_name, system.key_order_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot save the system configuration";
    return act_status;
  }

  // Send update msg
  ActSystemPatchUpdateMsg ws_msg(ActPatchUpdateActionEnum::kUpdate, system, true);
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_msg);

  return act_status;
}

}  // namespace core
}  // namespace act
