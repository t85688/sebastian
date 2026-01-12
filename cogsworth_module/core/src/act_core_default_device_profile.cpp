#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"

namespace act {
namespace core {
ACT_STATUS ActCore::InitDefaultDeviceProfiles() {
  ACT_STATUS_INIT();

  // Read device profiles from configuration folder
  QSet<ActDeviceProfile> default_device_profile_set;
  qint64 last_assigned_default_device_profile_id = -1;

  default_device_profile_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_DEFAULT_DEVICE_PROFILE_FOLDER);
  */
  QDir dir(GetDefaultDeviceProfilePath());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActDeviceProfile data;
      act_status = act::database::ReadFromDB(data, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: default device profile";
        return act_status;
      }

      // Assign data to output argument
      default_device_profile_set.insert(data);

      // Update the id to improve the first create performance
      last_assigned_default_device_profile_id = data.GetId();
    }
  }

  this->SetDefaultDeviceProfileSet(default_device_profile_set);
  this->last_assigned_default_device_profile_id_ = last_assigned_default_device_profile_id;

  return act_status;
}

}  // namespace core
}  // namespace act
