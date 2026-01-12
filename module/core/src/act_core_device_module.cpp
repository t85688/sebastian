#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {
ACT_STATUS ActCore::InitEthernetModules() {
  ACT_STATUS_INIT();

  // Read ethernet modules from configuration folder
  QMap<qint64, ActEthernetModule> eth_module_map;
  eth_module_map.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_ETHERNET_MODULE_FOLDER);
  */
  QDir dir(GetEthernetModulePath());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActEthernetModule data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: ethernet module";
        return act_status;
      }

      // Assign data to output argument
      eth_module_map.insert(data.GetId(), data);

      // Assign module name to id map
      eth_module_name_id_map_.insert(data.GetModuleName(), data.GetId());
    }
  }

  this->SetEthernetModuleMap(eth_module_map);

  return act_status;
}

ACT_STATUS ActCore::InitSFPModules() {
  ACT_STATUS_INIT();

  // Read SFP modules from configuration folder
  QMap<qint64, ActSFPModule> sfp_module_map;
  sfp_module_map.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_SFP_MODULE_FOLDER);
  */
  QDir dir(GetSfpModulePath());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActSFPModule data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: SFP module";
        return act_status;
      }

      // Assign data to output argument
      sfp_module_map.insert(data.GetId(), data);

      // Assign module name to id map
      sfp_module_name_id_map_.insert(data.GetModuleName(), data.GetId());
    }
  }

  this->SetSFPModuleMap(sfp_module_map);

  return act_status;
}

ACT_STATUS ActCore::InitPowerModules() {
  ACT_STATUS_INIT();

  // Read ethernet modules from configuration folder
  QMap<qint64, ActPowerModule> power_module_map;
  power_module_map.clear();
  power_module_name_id_map_.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_POWER_MODULE_FOLDER);
  */
  QDir dir(GetPowerModulePath());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActPowerModule data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: power module";
        return act_status;
      }

      // Assign data to output argument
      power_module_map.insert(data.GetId(), data);

      // Assign module name to id map
      power_module_name_id_map_.insert(data.GetModuleName(), data.GetId());
    }
  }

  this->SetPowerModuleMap(power_module_map);

  return act_status;
}

ACT_STATUS ActCore::GetEthernetModuleNameIdMap(QMap<QString, qint64> &eth_module_name_id_map) {
  ACT_STATUS_INIT();

  eth_module_name_id_map = this->eth_module_name_id_map_;

  return act_status;
}

ACT_STATUS ActCore::GetSFPModuleNameIdMap(QMap<QString, qint64> &sfp_module_name_id_map) {
  ACT_STATUS_INIT();

  sfp_module_name_id_map = this->sfp_module_name_id_map_;

  return act_status;
}

ACT_STATUS ActCore::GetPowerModuleNameIdMap(QMap<QString, qint64> &power_module_name_id_map) {
  ACT_STATUS_INIT();

  power_module_name_id_map = this->power_module_name_id_map_;

  return act_status;
}

}  // namespace core
}  // namespace act