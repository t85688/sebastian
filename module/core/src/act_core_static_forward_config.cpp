#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"
#include "act_deploy_path_data.hpp"
namespace act {
namespace core {

ACT_STATUS ActCore::GetUnicastStaticForwardConfig(qint64 &project_id, qint64 &device_id,
                                                  ActStaticForwardTable &static_forward_table) {
  ACT_STATUS_INIT();
  qDebug() << __func__;

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Found UnicastStaticForward table
  auto static_forward_table_map = project.GetDeviceConfig().GetUnicastStaticForwardTables();
  if (!static_forward_table_map.contains(device_id)) {  // not found
    return std::make_shared<ActStatusNotFound>(QString("UnicastStaticForwardConfig(Device:%1)").arg(device_id));
  }

  static_forward_table = static_forward_table_map[device_id];

  return act_status;
}

ACT_STATUS ActCore::GetMulticastStaticForwardConfig(qint64 &project_id, qint64 &device_id,
                                                    ActStaticForwardTable &static_forward_table) {
  ACT_STATUS_INIT();
  qDebug() << __func__;

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Found MulticastStaticForward table
  auto static_forward_table_map = project.GetDeviceConfig().GetMulticastStaticForwardTables();
  if (!static_forward_table_map.contains(device_id)) {  // not found
    return std::make_shared<ActStatusNotFound>(QString("MulticastStaticForwardConfig(Device:%1)").arg(device_id));
  }

  static_forward_table = static_forward_table_map[device_id];

  return act_status;
}

}  // namespace core
}  // namespace act
