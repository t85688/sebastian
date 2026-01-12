#include <QSet>

#include "act_core.hpp"
#include "act_southbound.hpp"

namespace act {
namespace core {
ACT_STATUS ActCore::InitSnmpGlobalResource() {
  ACT_STATUS_INIT();

  // Init SNMP global resource
  ActSouthbound south;
  act_status = south.InitSnmpResource();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "InitSnmpResource(): Initial SNMP resource failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::ClearSnmpGlobalResource() {
  ACT_STATUS_INIT();

  // Clear SNMP global resource
  ActSouthbound south;
  act_status = south.ClearSnmpResource();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ClearSnmpResource(): Clear SNMP resource failed";
    return act_status;
  }

  return act_status;
}

}  // namespace core
}  // namespace act
