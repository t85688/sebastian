#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::UpdateSFPCounts(qint64 &project_id, QMap<QString, quint32> sfp_counts) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  project.SetSFPCounts(sfp_counts);

  // Overwrite SKU quantities in the project
  ActUpdateSkuQuantityRequest sku_quantity_request;
  QMap<QString, quint64> sku_quantity_map;
  for (auto it = sfp_counts.begin(); it != sfp_counts.end(); ++it) {
    sku_quantity_map.insert(it.key(), it.value());
  }
  sku_quantity_request.SetSkuQuantity(sku_quantity_map);

  act_status = this->UpdateSkuQuantity(project, sku_quantity_request);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::PatchSFPCounts(qint64 &project_id, QMap<QString, quint32> sfp_counts) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  for (auto module_name : sfp_counts.keys()) {
    project.GetSFPCounts()[module_name] = sfp_counts[module_name];
  }

  // Overwrite SKU quantities in the project
  ActUpdateSkuQuantityRequest sku_quantity_request;
  QMap<QString, quint64> sku_quantity_map;
  for (auto it = project.GetSFPCounts().begin(); it != project.GetSFPCounts().end(); ++it) {
    sku_quantity_map.insert(it.key(), it.value());
  }
  sku_quantity_request.SetSkuQuantity(sku_quantity_map);

  act_status = this->UpdateSkuQuantity(project, sku_quantity_request);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::ClearSFPCounts(qint64 &project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  project.GetSFPCounts().clear();

  // Overwrite SKU quantities in the project
  ActSkuList sku_list;
  QList<QString> sku_list_map;
  for (auto it = project.GetSkuQuantitiesMap().begin(); it != project.GetSkuQuantitiesMap().end(); ++it) {
    sku_list_map.append(it.key());
  }
  sku_list.SetSkuList(sku_list_map);

  act_status = this->DeleteSkuQuantity(project, sku_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DeleteSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  return act_status;
}

}  // namespace core
}  // namespace act
