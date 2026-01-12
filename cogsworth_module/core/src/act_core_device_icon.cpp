#include <QSet>

#include "act_core.hpp"
#include "act_db.hpp"
#include "opcua_class_based_server.h"

namespace act {
namespace core {

ACT_STATUS ActCore::UpdateDeviceProfileIcon(qint64 &device_profile_id, QString &icon_name) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  QSet<ActDeviceProfile> device_profiles = this->GetDeviceProfileSet();

  // Check icon name is valid
  // Check if contains special characters (\ / : * ? “ < > |) that can not use in windows
  const QString forbidden_char = "\\/:*?\"<>|";
  for (const QChar &c : forbidden_char) {
    if (icon_name.contains(c)) {
      QString error_msg = QString("Model name contains forbidden characters, the name is %1").arg(icon_name);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  // Check if trailing space and multiple consecutive spaces
  if (icon_name.endsWith(" ") || icon_name.contains(QRegExp("\\s{2,}"))) {
    QString error_msg =
        QString("Model name contains multiple consecutive spaces or end with space, the name is %1").arg(icon_name);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Update the specific device profile's content
  ActDeviceProfile device_profile;
  act_status = ActGetItemById<ActDeviceProfile>(device_profiles, device_profile_id, device_profile);
  if (IsActStatusNotFound(act_status)) {
    qCritical() << __func__;
    qCritical() << "The device profile id" << QString::number(device_profile_id) << "matches no device profile.";
    return std::make_shared<ActStatusNotFound>(QString::number(device_profile_id));
  }

  // Check the item does exist by id
  typename QSet<ActDeviceProfile>::const_iterator iterator;
  iterator = device_profiles.find(device_profile);
  if (iterator != device_profiles.end()) {
    // If yes, delete it
    device_profiles.erase(iterator);
  }

  device_profile.SetIconName(icon_name);

  // Insert the device_profile to core set
  device_profiles.insert(device_profile);

  this->SetDeviceProfileSet(device_profiles);

  // Write to db
  act_status = this->UpdateDeviceProfile(device_profile);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Cannot save the device profile configuration";
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

}  // namespace core
}  // namespace act
