#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {
ACT_STATUS ActCore::InitFirmwareFeatureProfiles() {
  ACT_STATUS_INIT();

  // Read Firmware feature profiles from configuration folder
  QSet<ActFirmwareFeatureProfile> firmware_feature_profile_set;
  qint64 last_assigned_firmware_feature_profile_id = -1;

  firmware_feature_profile_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_FIRMWARE_FEATURE_PROFILE_FOLDER);
  */
  QDir dir(GetFirmwareFeatureProfile());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActFirmwareFeatureProfile data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: firmware feature profile";
        return act_status;
      }

      // Assign data to output argument
      firmware_feature_profile_set.insert(data);
      // Update the id to improve the first create performance
      last_assigned_firmware_feature_profile_id = data.GetId();
    }
  }

  this->SetFirmwareFeatureProfileSet(firmware_feature_profile_set);
  this->last_assigned_firmware_feature_profile_id_ = last_assigned_firmware_feature_profile_id;
  return act_status;
}

}  // namespace core
}  // namespace act
