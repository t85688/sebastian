#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {
ACT_STATUS ActCore::InitGeneralProfiles() {
  ACT_STATUS_INIT();

  // Read general profile from configuration folder
  QMap<QString, ActSkuWithPrice> general_profile_map;
  general_profile_map.clear();

  // Retrieve the data from database
  QDir dir(GetGeneralProfilePath());

  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActSkuWithPrice data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: general profile";
        return act_status;
      }

      // Assign data to output argument
      general_profile_map.insert(data.GetModelName(), data);
    }
  }

  this->SetGeneralProfileMap(general_profile_map);

  return act_status;
}

ACT_STATUS ActCore::GetSkuQuantities(const qint64 &project_id, ActSkuQuantities &sku_quantities) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetProject() failed";
    return act_status;
  }

  // Get the SKU quantity from the project
  sku_quantities.SetSkuQuantitiesMap(project.GetSkuQuantitiesMap());

  return act_status;
}

ACT_STATUS GetSkuCategory(const QString &model_name, ActL1CategoryEnum &category) {
  ACT_STATUS_INIT();

  const QMap<QString, ActSkuWithPrice> general_profiles = act::core::g_core.GetGeneralProfileMap();
  // TODO: when using IT DB, how to known the category is switch or accessory?
  if (general_profiles.contains(model_name)) {
    category = general_profiles[model_name].GetL1Category();
  } else {
    QString error_msg = QString("%1 is not in general profile.").arg(model_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  return act_status;
}

ACT_STATUS SplitSkuQuantityMap(ActUpdateSkuQuantityRequest &sku_quantity_request,
                               QMap<QString, quint64> &device_quantity_map,
                               QMap<QString, quint64> &accessory_quantity_map) {
  ACT_STATUS_INIT();

  // Split into device or accessory maps
  const auto &sku_map = sku_quantity_request.GetSkuQuantity();

  for (auto it = sku_map.begin(); it != sku_map.end(); ++it) {
    const QString &model_name = it.key();
    quint64 quantity = it.value();

    ActL1CategoryEnum category;
    act_status = GetSkuCategory(model_name, category);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "GetSkuCategory() failed";
      return act_status;
    }

    if (category == ActL1CategoryEnum::kAccessories) {
      accessory_quantity_map[model_name] += quantity;
    } else if (category == ActL1CategoryEnum::kSwitches) {
      device_quantity_map[model_name] += quantity;
    }
  }

  return act_status;
}

ACT_STATUS SplitSkuList(QList<QString> &sku_list, QList<QString> &device_list, QList<QString> &accessory_list) {
  ACT_STATUS_INIT();

  // Split into device or accessory list

  for (auto model_name : sku_list) {
    ActL1CategoryEnum category;
    act_status = GetSkuCategory(model_name, category);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "GetSkuCategory() failed";
      return act_status;
    }

    if (category == ActL1CategoryEnum::kAccessories) {
      accessory_list.append(model_name);
    } else if (category == ActL1CategoryEnum::kSwitches) {
      device_list.append(model_name);
    }
  }

  return act_status;
}

ACT_STATUS SendSkuQuantityUpdatedWs(ActProject &project, QMap<QString, quint64> &device_quantity_map,
                                    QMap<QString, quint64> &accessory_quantity_map) {
  ACT_STATUS_INIT();

  QMap<QString, ActSkuQuantity> project_sku_map = project.GetSkuQuantitiesMap();

  QMap<QString, ActSkuQuantity> sku_quantity_map;

  if (device_quantity_map.isEmpty() && accessory_quantity_map.isEmpty()) {
    // do not send ws
    return act_status;
  }

  // device_quantity_map -> sku_quantities
  for (auto it = device_quantity_map.begin(); it != device_quantity_map.end(); it++) {
    QString model_name = it.key();
    quint64 quantity = it.value();

    if (quantity == 0) {
      // deleted sku, skip
      continue;
    } else {
      sku_quantity_map[model_name] = project_sku_map[model_name];
    }
  }

  // accessory_quantity_map -> sku_quantities
  for (auto it = accessory_quantity_map.begin(); it != accessory_quantity_map.end(); it++) {
    QString model_name = it.key();
    quint64 quantity = it.value();

    if (quantity == 0) {
      // deleted sku, skip
      continue;
    } else {
      sku_quantity_map[model_name] = project_sku_map[model_name];
    }
  }

  ActSkuQuantities sku_quantities;
  sku_quantities.SetSkuQuantitiesMap(sku_quantity_map);

  auto project_id = project.GetId();
  ActSkuQuantityPatchUpdateMsg sku_quantity_ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, sku_quantities, true);
  act::core::g_core.SendMessageToListener(ActWSTypeEnum::kProject, false, sku_quantity_ws_msg, project_id);

  return act_status;
}

ACT_STATUS SendDeviceBagUpdatedWs(ActProject &project, QMap<QString, quint64> &device_quantity_map) {
  ACT_STATUS_INIT();

  QMap<QString, ActSkuQuantity> project_sku_map = project.GetSkuQuantitiesMap();
  auto device_profile_set = act::core::g_core.GetDeviceProfileSet();

  QMap<QString, ActDeviceBagItem> device_bag_map;

  // device_quantity_map -> device_bag_map & sku_quantities_map
  for (auto it = device_quantity_map.begin(); it != device_quantity_map.end(); it++) {
    QString model_name = it.key();
    quint64 quantity = it.value();

    ActDeviceBagItem device_bag_item;

    if (quantity == 0) {
      // deleted sku, skip
      continue;
    } else {
      qint64 remainingQuantity =
          project_sku_map[model_name].GetQuantity() - project_sku_map[model_name].GetInTopologyCount();
      device_bag_item.SetRemainingQuantity(remainingQuantity);
    }

    bool in_device_profile = device_profile_set.contains(ActDeviceProfile(model_name));
    device_bag_item.SetCanAddToTopology(in_device_profile);

    device_bag_map[model_name] = device_bag_item;
  }

  ActDeviceBag device_bag;
  device_bag.SetDeviceBagMap(device_bag_map);

  auto project_id = project.GetId();
  ActDeviceBagPatchUpdateMsg device_bag_ws_msg(ActPatchUpdateActionEnum::kUpdate, project_id, device_bag, true);
  act::core::g_core.SendMessageToListener(ActWSTypeEnum::kProject, false, device_bag_ws_msg, project_id);

  return act_status;
}

ACT_STATUS SendSkuAndBagUpdatedWs(ActProject &project, QMap<QString, quint64> &device_quantity_map,
                                  QMap<QString, quint64> &accessory_quantity_map) {
  ACT_STATUS_INIT();

  act_status = SendSkuQuantityUpdatedWs(project, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuQuantityUpdatedWs() failed";
    return act_status;
  }

  if (!device_quantity_map.isEmpty()) {
    act_status = SendDeviceBagUpdatedWs(project, device_quantity_map);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "SendDeviceBagUpdatedWs() failed";
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS SendSkuQuantityDeletedWs(ActProject &project, QList<QString> &device_list, QList<QString> &accessory_list) {
  ACT_STATUS_INIT();

  ActSkuList sku_list;
  sku_list.SetSkuList(device_list + accessory_list);

  if (!sku_list.GetSkuList().isEmpty()) {
    auto project_id = project.GetId();
    AckSkuQuantityDeleteMsg ws_msg(ActPatchUpdateActionEnum::kDelete, project_id, sku_list, true);
    act::core::g_core.SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);
  }

  return act_status;
}

ACT_STATUS SendDeviceBagDeletedWs(ActProject &project, QList<QString> &device_list) {
  ACT_STATUS_INIT();

  ActModelList model_list;
  model_list.SetModelList(device_list);

  auto project_id = project.GetId();
  AckDeviceBagDeleteMsg ws_msg(ActPatchUpdateActionEnum::kDelete, project_id, model_list, true);
  act::core::g_core.SendMessageToListener(ActWSTypeEnum::kProject, false, ws_msg, project_id);

  return act_status;
}

ACT_STATUS SendSkuAndBagDeletedWs(ActProject &project, QList<QString> &device_list, QList<QString> &accessory_list) {
  ACT_STATUS_INIT();

  act_status = SendSkuQuantityDeletedWs(project, device_list, accessory_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuQuantityDeletedWs() failed";
    return act_status;
  }

  if (!device_list.isEmpty()) {
    act_status = SendDeviceBagDeletedWs(project, device_list);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "SendDeviceBagDeletedWs() failed";
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateSkuQuantity(const qint64 &project_id, ActUpdateSkuQuantityRequest sku_quantity_request) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetProject() failed";
    return act_status;
  }

  // old_sku_map & new_sku_map are for deleted ws compare
  auto old_sku_map = project.GetSkuQuantitiesMap();
  QMap<QString, quint64> req_map = sku_quantity_request.GetSkuQuantity();
  // find missing keys, and insert into request map
  for (const QString &key : old_sku_map.keys()) {
    if (!req_map.contains(key)) {
      req_map[key] = 0;
    }
  }
  sku_quantity_request.SetSkuQuantity(req_map);

  act_status = this->UpdateSkuQuantity(project, sku_quantity_request);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateSkuQuantity() failed";
    return act_status;
  }

  // update the project to the database
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateProject() failed";
    return act_status;
  }

  // send ws notification for updated devices and accessories
  // sku_quantity_request -> device_quantity_map & accessory_quantity_map
  QMap<QString, quint64> device_quantity_map;
  QMap<QString, quint64> accessory_quantity_map;
  act_status = SplitSkuQuantityMap(sku_quantity_request, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SplitSkuQuantityMap() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  act_status = SendSkuAndBagUpdatedWs(project, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuAndBagUpdatedWs() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // send ws notification for deleted devices and accessories
  // quantity = 0
  QList<QString> deleted_device_list;
  QList<QString> deleted_accessory_list;
  for (auto it = device_quantity_map.begin(); it != device_quantity_map.end(); it++) {
    QString model_name = it.key();
    qint64 quantity = it.value();
    if (quantity == 0) {
      deleted_device_list.append(model_name);
    }
  }
  for (auto it = accessory_quantity_map.begin(); it != accessory_quantity_map.end(); it++) {
    QString model_name = it.key();
    qint64 quantity = it.value();
    if (quantity == 0) {
      deleted_accessory_list.append(model_name);
    }
  }

  if (deleted_device_list.isEmpty() && deleted_accessory_list.isEmpty()) {
    // no item to update
  } else {
    act_status = SendSkuAndBagDeletedWs(project, deleted_device_list, deleted_accessory_list);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "SendSkuAndBagDeletedWs() failed for project:" << project.GetProjectName().toStdString().c_str();
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateSkuQuantity(ActProject &project, ActUpdateSkuQuantityRequest sku_quantity_request) {
  ACT_STATUS_INIT();

  // Get the SKU quantity from the project
  QMap<QString, ActSkuQuantity> sku_quantities_map = project.GetSkuQuantitiesMap();

  QMap<QString, ActSkuWithPrice> general_profiles = this->GetGeneralProfileMap();
  QMap<QString, quint64> sku_quantity_request_map = sku_quantity_request.GetSkuQuantity();

  // valid before update sku map
  for (auto it = sku_quantity_request_map.begin(); it != sku_quantity_request_map.end(); it++) {
    QString sku_name = it.key();
    qint64 quantity = it.value();

    ActSkuWithPrice general_profile = general_profiles[sku_name];
    if (general_profile.GetModelName().isEmpty()) {
      qCritical() << "SKU model name" << sku_name << "not found in general profile";
      return std::make_shared<ActStatusNotFound>(sku_name);
    }

    // quantity can not be less than inTopologyCount (used devices in topology)
    qint64 in_topology_count = sku_quantities_map[sku_name].GetInTopologyCount();
    if (quantity < in_topology_count) {
      QString error_msg =
          QString("Quantity of %1 is less than used devices (%2) in topology.").arg(sku_name).arg(in_topology_count);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Update the SKU quantity, the sku_quantity is a map of SKU name and quantity
  // the body is key:value pair, key is SKU name, value is quantity
  for (auto it = sku_quantity_request_map.begin(); it != sku_quantity_request_map.end(); it++) {
    QString sku_name = it.key();
    qint64 quantity = it.value();

    ActSkuWithPrice general_profile = general_profiles[sku_name];

    if (quantity == 0) {
      // [bugfix:3973] Remove the SKU if quantity is 0
      sku_quantities_map.remove(sku_name);
      continue;
    }

    // Update the SKU quantity
    sku_quantities_map[sku_name].SetId(general_profile.GetId());
    sku_quantities_map[sku_name].SetIconName(general_profile.GetIconName());
    sku_quantities_map[sku_name].SetModelName(general_profile.GetModelName());
    sku_quantities_map[sku_name].SetDescription(general_profile.GetDescription());
    sku_quantities_map[sku_name].SetPrice(general_profile.GetPrice());
    sku_quantities_map[sku_name].SetCurrency(general_profile.GetCurrency());
    sku_quantities_map[sku_name].SetProfiles(general_profile.GetProfiles());

    sku_quantities_map[sku_name].SetQuantity(quantity);
    qint64 in_topology_count = sku_quantities_map[sku_name].GetInTopologyCount();
    sku_quantities_map[sku_name].SetInTopologyCount(in_topology_count);

    // total price format is "Currency Quantity * Price"
    sku_quantities_map[sku_name].SetTotalPrice(
        QString("%1 %2")
            .arg(GetStringFromEnum<ActCurrencyEnum>(general_profile.GetCurrency(), kActCurrencyEnumMap))
            .arg(QString::number(quantity * general_profile.GetPrice().toDouble())));
  }

  // Update the SKU quantity to the project
  project.SetSkuQuantitiesMap(sku_quantities_map);

  return act_status;
}

ACT_STATUS ActCore::DeleteSkuQuantity(const qint64 &project_id, ActSkuList &sku_list) {
  ACT_STATUS_INIT();

  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetProject() failed";
    return act_status;
  }

  act_status = this->DeleteSkuQuantity(project, sku_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DeleteSkuQuantity() failed";
    return act_status;
  }

  // update the project to the database
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "UpdateProject() failed";
    return act_status;
  }

  QList<QString> device_list;
  QList<QString> accessory_list;
  act_status = SplitSkuList(sku_list.GetSkuList(), device_list, accessory_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SplitSkuList() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  act_status = SendSkuAndBagDeletedWs(project, device_list, accessory_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuQuantityDeletedWs() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::DeleteSkuQuantity(ActProject &project, const ActSkuList &sku_list) {
  ACT_STATUS_INIT();

  // Get the SKU quantity from the project
  QMap<QString, ActSkuQuantity> sku_quantities_map = project.GetSkuQuantitiesMap();

  QList<QString> model_name_list = sku_list.GetSkuList();

  // valid before delete sku
  for (auto it = model_name_list.begin(); it != model_name_list.end(); it++) {
    QString sku_name = *it;

    if (!sku_quantities_map.contains(sku_name)) {
      qCritical() << "SKU name" << sku_name << "not found in project";
      return std::make_shared<ActStatusNotFound>(sku_name);
    }

    // cannot delete SKU if devices exist in topology
    if (sku_quantities_map[sku_name].GetInTopologyCount() != 0) {
      QString error_msg = QString("Cannot delete SKU %1: device(s) still exist in topology.").arg(sku_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Delete the SKU quantity, the sku_list is a list of SKU name
  // the body is a list of SKU name
  for (auto it = model_name_list.begin(); it != model_name_list.end(); it++) {
    QString sku_name = *it;

    // Delete the SKU quantity
    sku_quantities_map.remove(sku_name);
  }

  // Update the SKU quantity to the project
  project.SetSkuQuantitiesMap(sku_quantities_map);

  return act_status;
}

ACT_STATUS GenerateSkuQuantityFromDevice(const ActDevice &device, QMap<QString, quint64> &device_quantity_map,
                                         QMap<QString, quint64> &accessory_quantity_map) {
  ACT_STATUS_INIT();

  // Update SKU quantities with device model and module information
  QString device_model_name = device.GetDeviceProperty().GetModelName();

  // Update device count
  device_quantity_map[device_model_name] = 1;

  QMap<qint64, ActEthernetModule> eth_module_map = act::core::g_core.GetEthernetModuleMap();
  QMap<qint64, ActPowerModule> pwr_module_map = act::core::g_core.GetPowerModuleMap();

  // handle Ethernet module
  for (const auto &module_id : device.GetModularConfiguration().GetEthernet().values()) {
    if (eth_module_map.contains(module_id)) {
      QString module_name = eth_module_map[module_id].GetModuleName();
      if (accessory_quantity_map.contains(module_name)) {
        accessory_quantity_map[module_name] += 1;  // Update module count
      } else {
        accessory_quantity_map[module_name] = 1;  // Update module count
      }
    } else {
      QString error_msg = QString("Ethernet module id %1 not found").arg(module_id);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("Ethernet module id %1").arg(module_id));
    }
  }

  // handle power module
  for (const auto &module_id : device.GetModularConfiguration().GetPower().values()) {
    if (pwr_module_map.contains(module_id)) {
      ActPowerModule module = pwr_module_map[module_id];
      if (module.GetBuiltIn()) {
        continue;
      }
      QString module_name = module.GetModuleName();
      if (accessory_quantity_map.contains(module_name)) {
        accessory_quantity_map[module_name] += 1;  // Update module count
      } else {
        accessory_quantity_map[module_name] = 1;  // Update module count
      }
    } else {
      QString error_msg = QString("Power module id %1 not found").arg(module_id);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusNotFound>(QString("Power module id %1").arg(module_id));
    }
  }

  return act_status;
}

/**
 * @brief EnsureModules
 *
 * Ensures that a given SKU (module) exists in the project SKU map and its quantity is sufficient.
 *
 * Functionality:
 * 1. If the SKU does not exist in project_sku_map, it will create a new ActSkuQuantity
 *    and set its quantity and in_topology_count to added_in_topology_count.
 * 2. If the SKU already exists, it checks whether its quantity is less than the
 *    current in_topology_count plus added_in_topology_count. If insufficient, it
 *    increases the quantity to match.
 * 3. Updates the total price of the SKU automatically.
 *
 * @param project_sku_map Map of all SKUs in the project (key: SKU name, value: ActSkuQuantity)
 * @param sku_name The SKU to ensure/add
 * @param added_in_topology_count The number of this SKU being added to the topology
 * @return ACT_STATUS
 */
ACT_STATUS EnsureModules(QMap<QString, ActSkuQuantity> &project_sku_map, const QString &sku_name,
                         const qint64 &added_in_topology_count) {
  ACT_STATUS_INIT();

  // Get the general SKU profile from the global profile map
  QMap<QString, ActSkuWithPrice> general_profiles = act::core::g_core.GetGeneralProfileMap();
  if (!general_profiles.contains(sku_name)) {
    QString error_msg = QString("SKU '%1' not found in general profiles").arg(sku_name);
    qCritical() << error_msg;
    return std::make_shared<ActBadRequest>(error_msg);
  }
  ActSkuWithPrice general_profile = general_profiles[sku_name];

  // If the SKU does not exist in the project map, create a new one
  if (!project_sku_map.contains(sku_name)) {
    ActSkuQuantity new_sku;

    // Initialize SKU information
    new_sku.SetId(general_profile.GetId());
    new_sku.SetIconName(general_profile.GetIconName());
    new_sku.SetModelName(sku_name);
    new_sku.SetDescription(general_profile.GetDescription());
    new_sku.SetPrice(general_profile.GetPrice());
    new_sku.SetCurrency(general_profile.GetCurrency());
    new_sku.SetProfiles(general_profile.GetProfiles());

    // Set quantity and in_topology_count
    new_sku.SetQuantity(added_in_topology_count);
    new_sku.SetInTopologyCount(added_in_topology_count);

    // Calculate total price
    new_sku.SetTotalPrice(
        QString("%1 %2")
            .arg(GetStringFromEnum<ActCurrencyEnum>(general_profile.GetCurrency(), kActCurrencyEnumMap))
            .arg(QString::number(added_in_topology_count * general_profile.GetPrice().toDouble())));

    // Insert the new SKU into the project map
    project_sku_map.insert(sku_name, new_sku);

  } else {
    // SKU already exists → check quantity and adjust if necessary
    ActSkuQuantity &sku = project_sku_map[sku_name];

    qint64 current_quantity = sku.GetQuantity();
    qint64 current_in_topology = sku.GetInTopologyCount();
    qint64 new_in_topology = current_in_topology + added_in_topology_count;

    // If quantity is less than the new in_topology_count, increase it
    if (current_quantity < new_in_topology) {
      qint64 delta = new_in_topology - current_quantity;
      sku.SetQuantity(current_quantity + delta);
    }

    // Update in_topology_count
    sku.SetInTopologyCount(new_in_topology);

    // Update total price
    sku.SetTotalPrice(QString("%1 %2")
                          .arg(GetStringFromEnum<ActCurrencyEnum>(general_profile.GetCurrency(), kActCurrencyEnumMap))
                          .arg(QString::number(sku.GetQuantity() * general_profile.GetPrice().toDouble())));
  }

  return act_status;
}

/**
 * @brief IncreaseSkuQuantity
 *
 * Updates the quantities of SKUs in a project based on a given quantity map.
 * Handles both adding new SKUs and updating existing ones, and ensures the
 * integrity of in_topology_count vs. total quantity.
 *
 * Functionality:
 * 1. Iterates over each SKU in quantity_map and determines if it is an accessory.
 * 2. For accessories:
 *    - If the SKU exists, calls EnsureModules to ensure quantity matches the in_topology_count.
 *    - If the SKU does not exist, automatically adds it via EnsureModules.
 * 3. For non-accessories:
 *    - If from_bag is true and SKU does not exist, returns an error.
 *    - Otherwise, updates quantity and in_topology_count.
 * 4. Updates SKU information (ID, price, currency, profiles) and recalculates total price.
 *
 * @param project The project containing the SKU map
 * @param quantity_map Map of SKU names and the number to add (key: SKU name, value: quantity)
 * @param from_bag Indicates whether the SKUs are being added from a device bag
 * @return ACT_STATUS
 */
ACT_STATUS IncreaseSkuQuantity(ActProject &project, QMap<QString, quint64> &quantity_map, const bool from_bag) {
  ACT_STATUS_INIT();

  // Get the general profile map for all SKUs
  QMap<QString, ActSkuWithPrice> general_profiles = act::core::g_core.GetGeneralProfileMap();
  // Get the SKU quantity from the project
  QMap<QString, ActSkuQuantity> sku_quantities_map = project.GetSkuQuantitiesMap();

  // Update the SKU quantity, the quantity_map is a map of SKU name and quantity
  // the body is key:value pair, key is SKU name, value is quantity
  for (auto it = quantity_map.begin(); it != quantity_map.end(); it++) {
    QString sku_name = it.key();
    qint64 added_in_topology_count = it.value();

    // Determine the SKU category (accessory or not)
    ActL1CategoryEnum l1_category;
    GetSkuCategory(sku_name, l1_category);
    bool is_accessory = (l1_category == ActL1CategoryEnum::kAccessories);

    // check SKU is exists
    bool sku_exists = sku_quantities_map.contains(sku_name);

    // Non-accessories from bag cannot be added if they do not exist
    if (!is_accessory && from_bag && !sku_exists) {
      QString error_msg = QString("Cannot add SKU '%1' from bag: SKU does not exist in project").arg(sku_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // from bag: do not add quantity, just update in_topology_count
    if (from_bag) {
      // accessories (modules in device) may need to be ensured
      if (is_accessory) {
        act_status = EnsureModules(sku_quantities_map, sku_name, added_in_topology_count);
        if (!IsActStatusSuccess(act_status)) {
          qCritical() << "EnsureModules() failed. module is: " << sku_name.toStdString().c_str();
          return act_status;
        }
      } else if (!is_accessory && sku_exists) {
        // not accessory, just update in_topology_count
        ActSkuQuantity &sku = sku_quantities_map[sku_name];
        sku.SetInTopologyCount(sku.GetInTopologyCount() + added_in_topology_count);
      }
    }

    // not from bag: just increase quantity & in_topology_count
    if (!from_bag) {
      if (sku_exists) {
        // update quantity & in_topology_count
        ActSkuQuantity &sku = sku_quantities_map[sku_name];
        sku.SetQuantity(sku.GetQuantity() + added_in_topology_count);
        sku.SetInTopologyCount(sku.GetInTopologyCount() + added_in_topology_count);

      } else {
        // create a new sku
        ActSkuQuantity new_sku;
        ActSkuWithPrice general_profile = general_profiles[sku_name];

        new_sku.SetId(general_profile.GetId());
        new_sku.SetIconName(general_profile.GetIconName());
        new_sku.SetModelName(sku_name);
        new_sku.SetDescription(general_profile.GetDescription());
        new_sku.SetPrice(general_profile.GetPrice());
        new_sku.SetCurrency(general_profile.GetCurrency());
        new_sku.SetProfiles(general_profile.GetProfiles());

        // Set quantity and in_topology_count
        new_sku.SetQuantity(added_in_topology_count);
        new_sku.SetInTopologyCount(added_in_topology_count);

        // Calculate total price
        new_sku.SetTotalPrice(
            QString("%1 %2")
                .arg(GetStringFromEnum<ActCurrencyEnum>(general_profile.GetCurrency(), kActCurrencyEnumMap))
                .arg(QString::number(added_in_topology_count * general_profile.GetPrice().toDouble())));

        // Insert the new SKU into the project map
        sku_quantities_map.insert(sku_name, new_sku);
      }
    }
  }

  // update to project
  project.SetSkuQuantitiesMap(sku_quantities_map);

  return act_status;
}

ACT_STATUS DecreaseSkuQuantity(ActProject &project, QMap<QString, quint64> &quantity_map, const bool to_bag) {
  ACT_STATUS_INIT();

  QMap<QString, ActSkuWithPrice> general_profiles = act::core::g_core.GetGeneralProfileMap();

  // Get the SKU quantity from the project
  QMap<QString, ActSkuQuantity> sku_quantities_map = project.GetSkuQuantitiesMap();

  // Update the SKU quantity, the quantity_map is a map of SKU name and quantity
  // the body is key:value pair, key is SKU name, value is quantity
  for (auto it = quantity_map.begin(); it != quantity_map.end(); it++) {
    QString sku_name = it.key();
    qint64 removed_in_topology_count = it.value();

    // to bag do not minus quantity, just update in_topology_count
    qint64 removed_quantity = 0;
    if (to_bag) {
      removed_quantity = 0;
    } else {
      removed_quantity = it.value();
    }

    // check SKU is exists
    if (!sku_quantities_map.contains(sku_name)) {
      qCritical() << "Cannot remove SKU" << sku_name << ": not found in project";
      return std::make_shared<ActStatusNotFound>(sku_name);
    }

    // total quantity & total in topology count
    qint64 total_quantity = sku_quantities_map[sku_name].GetQuantity() - removed_quantity;
    qint64 total_in_topology_count = sku_quantities_map[sku_name].GetInTopologyCount() - removed_in_topology_count;

    if (total_quantity < 0) {
      QString error_msg = QString("SKU quantity for %1 cannot be negative").arg(sku_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusInternalError>(error_msg);
    }
    if (total_in_topology_count < 0) {
      QString error_msg = QString("in topology count (used device) for %1 cannot be negative").arg(sku_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActStatusInternalError>(error_msg);
    }

    // update SKU
    if (total_quantity == 0) {
      sku_quantities_map.remove(sku_name);
      continue;
    } else {
      ActSkuWithPrice general_profile = general_profiles[sku_name];

      sku_quantities_map[sku_name].SetQuantity(total_quantity);
      sku_quantities_map[sku_name].SetInTopologyCount(total_in_topology_count);
      // total price format is "Currency Quantity * Price"
      sku_quantities_map[sku_name].SetTotalPrice(
          QString("%1 %2")
              .arg(GetStringFromEnum<ActCurrencyEnum>(general_profile.GetCurrency(), kActCurrencyEnumMap))
              .arg(QString::number(total_quantity * general_profile.GetPrice().toDouble())));
    }
  }

  project.SetSkuQuantitiesMap(sku_quantities_map);

  return act_status;
}

ACT_STATUS ActCore::UpdateSkuQuantityByAddDevice(ActProject &project, const ActDevice &device, const bool from_bag) {
  ACT_STATUS_INIT();

  QMap<QString, quint64> device_quantity_map;
  QMap<QString, quint64> accessory_quantity_map;

  auto general_profile_map = this->GetGeneralProfileMap();
  QString model_name = device.GetDeviceProperty().GetModelName();
  // device not in general profile -> can not add from bag
  if (!general_profile_map.contains(model_name) && from_bag) {
    QString error_msg = QString("Cannot add %1 from the bag").arg(model_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  // device not in general profile -> not add into SKU map
  if (!general_profile_map.contains(model_name)) {
    return act_status;
  }

  act_status = GenerateSkuQuantityFromDevice(device, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GenerateSkuQuantityFromDevice() failed for project:"
                << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // increase device SKU quantity
  act_status = IncreaseSkuQuantity(project, device_quantity_map, from_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "IncreaseSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // increase modules of device SKU quantity
  // use in-bag module first, if they are not enough, more will be added automatically until sufficient
  act_status = IncreaseSkuQuantity(project, accessory_quantity_map, from_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "IncreaseSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // send ws notification
  act_status = SendSkuAndBagUpdatedWs(project, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuAndBagUpdatedWs() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateSkuQuantityByDeleteDevice(ActProject &project, const ActDevice &device, const bool to_bag) {
  ACT_STATUS_INIT();

  QMap<QString, quint64> device_quantity_map;
  QMap<QString, quint64> accessory_quantity_map;

  // device not in general profile -> can not back to bag
  auto general_profile_map = this->GetGeneralProfileMap();
  QString model_name = device.GetDeviceProperty().GetModelName();
  if (!general_profile_map.contains(model_name) && to_bag) {
    QString error_msg = QString("%1 cannot return to the bag").arg(model_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActStatusInternalError>(error_msg);
  }

  // device not in general profile -> do not need to adjust quantity
  if (!general_profile_map.contains(model_name)) {
    return act_status;
  }

  act_status = GenerateSkuQuantityFromDevice(device, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GenerateSkuQuantityFromDevice() failed for project:"
                << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // decrease device SKU quantity
  act_status = DecreaseSkuQuantity(project, device_quantity_map, to_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DecreaseSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // decrease modules of device SKU quantity
  act_status = DecreaseSkuQuantity(project, accessory_quantity_map, to_bag);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "DecreaseSkuQuantity() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // send ws notification
  auto project_sku_quantity_map = project.GetSkuQuantitiesMap();
  QList<QString> deleted_device_list;
  QList<QString> deleted_accessory_list;

  for (auto it = device_quantity_map.begin(); it != device_quantity_map.end(); it++) {
    QString model_name = it.key();

    if (project_sku_quantity_map.contains(model_name)) {
      // get device quantity in project
      auto project_sku = project_sku_quantity_map[model_name];
      device_quantity_map[model_name] = project_sku.GetQuantity();
    } else {
      // device quantity is 0 in project
      deleted_device_list.append(model_name);
    }
  }
  for (auto it = accessory_quantity_map.begin(); it != accessory_quantity_map.end(); it++) {
    QString model_name = it.key();

    if (project_sku_quantity_map.contains(model_name)) {
      // get accessory quantity in project
      auto project_sku = project_sku_quantity_map[model_name];
      accessory_quantity_map[model_name] = project_sku.GetQuantity();
    } else {
      // accessory quantity is 0 in project
      deleted_accessory_list.append(model_name);
    }
  }

  act_status = SendSkuAndBagUpdatedWs(project, device_quantity_map, accessory_quantity_map);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuAndBagUpdatedWs() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }
  // if delete last device
  act_status = SendSkuAndBagDeletedWs(project, deleted_device_list, deleted_accessory_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuAndBagDeletedWs() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS CalculateDelta(ActProject &project, const ActDevice &old_device, const ActDevice &new_device,
                          ActSkuQuantityDelta &delta) {
  ACT_STATUS_INIT();

  auto general_profile_map = act::core::g_core.GetGeneralProfileMap();
  QMap<QString, ActSkuQuantity> project_sku_map = project.GetSkuQuantitiesMap();

  // diff model name
  QString old_model_name = old_device.GetDeviceProperty().GetModelName();
  QString new_model_name = new_device.GetDeviceProperty().GetModelName();

  QList<QString> deleted_devices = delta.GetDeletedDevices();
  QList<QString> deleted_accessories = delta.GetDeletedAccessories();
  QMap<QString, qint64> device_delta = delta.GetDeviceDelta();
  QMap<QString, qint64> accessory_delta = delta.GetAccessoryDelta();

  if (old_model_name != new_model_name) {
    // old model name --

    // old model name in general profile, delta -1
    if (general_profile_map.contains(old_model_name)) {
      if (project_sku_map[old_model_name].GetQuantity() == 1) {
        // last one to delete -> add into deleted device list (for delete ws)
        deleted_devices.append(old_model_name);
      } else {
        device_delta[old_model_name] = -1;
      }
    }

    // new model name in general profile, delta +1
    if (general_profile_map.contains(new_model_name)) {
      device_delta[new_model_name] = 1;
    }
  }

  // diff modules
  ActDeviceModularConfiguration old_device_modules = old_device.GetModularConfiguration();
  ActDeviceModularConfiguration new_device_modules = new_device.GetModularConfiguration();

  if (old_device_modules == new_device_modules) {
    // same, do nothing
  } else {
    QMap<QString, quint64> old_device_map, old_accessory_map;
    GenerateSkuQuantityFromDevice(old_device, old_device_map, old_accessory_map);
    QMap<QString, quint64> new_device_map, new_accessory_map;
    GenerateSkuQuantityFromDevice(new_device, new_device_map, new_accessory_map);

    // removed modules
    for (auto it = old_accessory_map.constBegin(); it != old_accessory_map.constEnd(); it++) {
      QString module_name = it.key();
      quint64 old_quantity = it.value();
      // if key (model name) is exist -> return value; if key is not exist -> return 0
      quint64 new_quantity = new_accessory_map.value(module_name, 0);

      // old module is not in general profile -> skip
      if (!general_profile_map.contains(module_name)) {
        continue;
      }

      // no model name in new accessory map
      if (new_quantity == 0) {
        if (project_sku_map[module_name].GetQuantity() == old_quantity) {
          deleted_accessories.append(module_name);
        } else {
          accessory_delta[module_name] = -static_cast<qint64>(old_quantity);
        }
      }
    }

    // module quantity differences (addition or partial reduction)
    for (auto it = new_accessory_map.constBegin(); it != new_accessory_map.constEnd(); it++) {
      QString module_name = it.key();
      quint64 new_quantity = it.value();
      quint64 old_quantity = old_accessory_map.value(module_name, 0);

      // new module is not in general profile -> skip
      if (!general_profile_map.contains(module_name)) {
        continue;
      }

      qint64 diff = static_cast<qint64>(new_quantity) - static_cast<qint64>(old_quantity);
      if (diff != 0) {
        accessory_delta[module_name] = accessory_delta[module_name] + diff;
      }
    }
  }

  delta.SetDeletedDevices(deleted_devices);
  delta.SetDeletedAccessories(deleted_accessories);
  delta.SetDeviceDelta(device_delta);
  delta.SetAccessoryDelta(accessory_delta);

  return act_status;
}

// TODO: deleted module go back to bag, future version will ask user to delete or go to bag
ACT_STATUS ApplySkuQuantityDelta(ActProject &project, ActSkuQuantityDelta &delta) {
  ACT_STATUS_INIT();

  QMap<QString, ActSkuQuantity> project_sku_map = project.GetSkuQuantitiesMap();

  QList<QString> deleted_devices = delta.GetDeletedDevices();
  QList<QString> deleted_accessories = delta.GetDeletedAccessories();
  QMap<QString, qint64> device_delta = delta.GetDeviceDelta();
  QMap<QString, qint64> accessory_delta = delta.GetAccessoryDelta();

  // deleted items
  if (!deleted_devices.isEmpty()) {
    QMap<QString, quint64> deleted_map;

    for (const QString &model_name : deleted_devices) {
      ActSkuQuantity sku = project_sku_map[model_name];
      deleted_map[model_name] = sku.GetQuantity();
    }

    act_status = DecreaseSkuQuantity(project, deleted_map, false);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "DecreaseSkuQuantity() failed (deleted items)";
      return act_status;
    }
  }

  if (!deleted_accessories.isEmpty()) {
    QMap<QString, quint64> deleted_map;
    for (const QString &model_name : deleted_accessories) {
      ActSkuQuantity sku = project_sku_map[model_name];
      deleted_map[model_name] = sku.GetQuantity();
    }

    act_status = DecreaseSkuQuantity(project, deleted_map, true);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "DecreaseSkuQuantity() failed (deleted items)";
      return act_status;
    }
  }

  // update items
  if (!device_delta.isEmpty() || !accessory_delta.isEmpty()) {
    QMap<QString, quint64> increase_device_map, decrease_device_map;

    for (auto it = device_delta.constBegin(); it != device_delta.constEnd(); ++it) {
      if (it.value() > 0) {
        increase_device_map[it.key()] += static_cast<quint64>(it.value());
      } else if (it.value() < 0) {
        decrease_device_map[it.key()] += static_cast<quint64>(-it.value());
      }
    }

    if (!increase_device_map.isEmpty()) {
      act_status = IncreaseSkuQuantity(project, increase_device_map, false);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "IncreaseSkuQuantity() failed";
        return act_status;
      }
    }

    if (!decrease_device_map.isEmpty()) {
      act_status = DecreaseSkuQuantity(project, decrease_device_map, false);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "DecreaseSkuQuantity() failed";
        return act_status;
      }
    }

    QMap<QString, quint64> increase_accessory_map, decrease_accessory_map;

    for (auto it = accessory_delta.constBegin(); it != accessory_delta.constEnd(); ++it) {
      if (it.value() > 0) {
        increase_accessory_map[it.key()] += static_cast<quint64>(it.value());
      } else if (it.value() < 0) {
        decrease_accessory_map[it.key()] += static_cast<quint64>(-it.value());
      }
    }

    if (!increase_accessory_map.isEmpty()) {
      act_status = IncreaseSkuQuantity(project, increase_accessory_map, true);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "IncreaseSkuQuantity() failed";
        return act_status;
      }
    }

    if (!decrease_accessory_map.isEmpty()) {
      act_status = DecreaseSkuQuantity(project, decrease_accessory_map, true);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "DecreaseSkuQuantity() failed";
        return act_status;
      }
    }

    // TODO: decrease will ask user to choose back to bag or not
    // // deleted items
    // if (!deleted_devices.isEmpty() || !deleted_accessories.isEmpty()) {
    //   QMap<QString, quint64> deleted_map;
    //   ActSkuQuantity sku = project_sku_map[model_name];
    //   for (const QString &model_name : delta.deletedDevices) {
    //     deleted_map[model_name] = sku.GetQuantity();
    //   }
    //   for (const QString &model_name : delta.deletedAccessories) {
    //     deleted_map[model_name] = sku.GetQuantity();
    //   }

    //   act_status = DecreaseSkuQuantity(project, deleted_map, false);
    //   if (!IsActStatusSuccess(act_status)) {
    //     qCritical() << "DecreaseSkuQuantity() failed (deleted items)";
    //     return act_status;
    //   }
    // }

    // // update items
    // if (!device_delta.isEmpty() || !accessory_delta.isEmpty()) {
    //   QMap<QString, quint64> increase_map, decrease_map;

    //   // delta -> increase map & decrease map
    //   auto processDelta = [&](const QMap<QString, qint64> &src) {
    //     for (auto it = src.constBegin(); it != src.constEnd(); ++it) {
    //       if (it.value() > 0) {
    //         increase_map[it.key()] += static_cast<quint64>(it.value());
    //       } else if (it.value() < 0) {
    //         decrease_map[it.key()] += static_cast<quint64>(-it.value());
    //       }
    //     }
    //   };

    //   processDelta(device_delta);
    //   processDelta(accessory_delta);

    //   if (!increase_map.isEmpty()) {
    //     act_status = IncreaseSkuQuantity(project, increase_map, false);
    //     if (!IsActStatusSuccess(act_status)) {
    //       qCritical() << "IncreaseSkuQuantity() failed";
    //       return act_status;
    //     }
    //   }

    //   if (!decrease_map.isEmpty()) {
    //     act_status = DecreaseSkuQuantity(project, decrease_map, false);
    //     if (!IsActStatusSuccess(act_status)) {
    //       qCritical() << "DecreaseSkuQuantity() failed";
    //       return act_status;
    //     }
    //   }
  }

  return act_status;
}

ACT_STATUS SendSkuQuantityDeltaWs(ActProject &project, ActSkuQuantityDelta &delta) {
  ACT_STATUS_INIT();

  QList<QString> deleted_devices = delta.GetDeletedDevices();
  QList<QString> deleted_accessories = delta.GetDeletedAccessories();
  QMap<QString, qint64> device_delta = delta.GetDeviceDelta();
  QMap<QString, qint64> accessory_delta = delta.GetAccessoryDelta();

  if (!deleted_devices.isEmpty() || !deleted_accessories.isEmpty()) {
    SendSkuAndBagDeletedWs(project, deleted_devices, deleted_accessories);
  }

  if (!device_delta.isEmpty() || !accessory_delta.isEmpty()) {
    QMap<QString, quint64> device_map, accessory_map;

    for (auto it = device_delta.constBegin(); it != device_delta.constEnd(); ++it) {
      device_map[it.key()] = static_cast<quint64>(qAbs(it.value()));
    }
    for (auto it = accessory_delta.constBegin(); it != accessory_delta.constEnd(); ++it) {
      accessory_map[it.key()] = static_cast<quint64>(qAbs(it.value()));
    }

    SendSkuAndBagUpdatedWs(project, device_map, accessory_map);
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateSkuQuantityByUpdateDevice(ActProject &project, const ActDevice &old_device,
                                                    const ActDevice &new_device) {
  ACT_STATUS_INIT();

  // diff old_device and new_device
  ActSkuQuantityDelta delta;
  act_status = CalculateDelta(project, old_device, new_device, delta);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "CalculateDelta() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // apply SKU quantity delta to project
  act_status = ApplySkuQuantityDelta(project, delta);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "ApplySkuQuantityDelta() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  // send ws
  act_status = SendSkuQuantityDeltaWs(project, delta);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendSkuQuantityDeltaWs() failed for project:" << project.GetProjectName().toStdString().c_str();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateSkuQuantityPriceInProjects() {
  ACT_STATUS_INIT();

  QSet<ActProject> project_set = this->GetProjectSet();

  // Update the SKU quantity price in all projects
  for (auto it = project_set.begin(); it != project_set.end(); it++) {
    ActProject project = *it;

    QMap<QString, ActSkuQuantity> &sku_quantities_map = project.GetSkuQuantitiesMap();
    // Update the price to each SKU quantity
    for (auto it = sku_quantities_map.begin(); it != sku_quantities_map.end(); ++it) {
      const QString &sku_name = it.key();
      ActSkuQuantity &sku_quantity = it.value();

      if (this->GetGeneralProfileMap().contains(sku_name)) {
        sku_quantity.SetPrice(this->GetGeneralProfileMap()[sku_name].GetPrice());
        sku_quantity.SetTotalPrice(
            QString("%1 %2")
                .arg(GetStringFromEnum<ActCurrencyEnum>(this->GetGeneralProfileMap()[sku_name].GetCurrency(),
                                                        kActCurrencyEnumMap))
                .arg(QString::number(sku_quantity.GetQuantity() *
                                     this->GetGeneralProfileMap()[sku_name].GetPrice().toDouble())));
      }
    }

    act_status = this->UpdateProject(project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "UpdateProject() failed";
      return act_status;
    }
  }

  return act_status;
}

}  // namespace core
}  // namespace act