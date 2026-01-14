#include <QSet>

#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::UpdateCycleSetting(qint64 &project_id, ActCycleSetting &cycle_setting) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateCycleSetting(project, cycle_setting);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update cycle setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Update project failed with project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActCycleSetting ws_cycle_setting(cycle_setting);
  ActCycleSettingPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project_id, ws_cycle_setting, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateCycleSetting(ActProject &project, ActCycleSetting &cycle_setting) {
  ACT_STATUS_INIT();

  // Currently, the admin base time should be zero
  if (cycle_setting.GetAdminBaseTime().GetSecond()) {
    QString error_msg = QString("Project (%1) - The admin base time %2 should be zero in the current version")
                            .arg(project.GetProjectName().toStdString().c_str())
                            .arg(cycle_setting.GetAdminBaseTime().GetSecond());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check the cycle setting
  quint64 sum = 0;
  bool best_effort_found = false;
  bool time_sync_found = false;
  bool n_a_found = false;

  // Check each period should be 125,000 ~ 999,999,999 ns
  for (auto time_slot : cycle_setting.GetTimeSlots()) {
    // The "N/A" time slot shouldn't be put in front of others
    // Which means once N/A is disappeared, there should be not have another type after that
    if (n_a_found && time_slot.GetTrafficType() != ActTimeSlotTrafficTypeEnum::kNA) {
      QString error_msg =
          QString("Project (%1) - The N/A time slot shouldn't be put in front of others").arg(project.GetProjectName());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (best_effort_found && time_slot.GetTrafficType() == ActTimeSlotTrafficTypeEnum::kBestEffort) {
      QString error_msg = QString("Project (%1) - There should be just one Best Effort time slot in the system")
                              .arg(project.GetProjectName());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (time_sync_found && time_slot.GetTrafficType() == ActTimeSlotTrafficTypeEnum::kTimeSync) {
      QString error_msg = QString("Project (%1) - There should be just one Time Sync time slot in the system")
                              .arg(project.GetProjectName());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    if (time_slot.GetTrafficType() == ActTimeSlotTrafficTypeEnum::kBestEffort) {
      if (time_slot.GetPeriod() < ACT_PERIOD_MIN || time_slot.GetPeriod() > ACT_PERIOD_MAX) {
        QString error_msg = QString("Project (%1) - The period in the Best Effort should be %2 ~ %3 ns")
                                .arg(project.GetProjectName())
                                .arg(ACT_PERIOD_MIN)
                                .arg(ACT_PERIOD_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
      best_effort_found = true;
    } else if (time_slot.GetTrafficType() == ActTimeSlotTrafficTypeEnum::kTimeSync) {
      if (time_slot.GetPeriod() < ACT_PERIOD_MIN || time_slot.GetPeriod() > ACT_PERIOD_MAX) {
        QString error_msg = QString("Project (%1) - The period in the Time Sync should be %2 ~ %3 ns")
                                .arg(project.GetProjectName())
                                .arg(ACT_PERIOD_MIN)
                                .arg(ACT_PERIOD_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
      time_sync_found = true;
    } else if (time_slot.GetTrafficType() == ActTimeSlotTrafficTypeEnum::kNA) {
      if (time_slot.GetPeriod() != 0) {
        QString error_msg = QString("Project (%1) - The period in N/A should be 0 ns").arg(project.GetProjectName());
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
      n_a_found = true;
    } else if (time_slot.GetTrafficType() == ActTimeSlotTrafficTypeEnum::kCyclic) {
      if (time_slot.GetPeriod() < ACT_PERIOD_MIN || time_slot.GetPeriod() > ACT_PERIOD_MAX) {
        QString error_msg = QString("Project (%1) - The period in the Cyclic should be %2 ~ %3 ns")
                                .arg(project.GetProjectName())
                                .arg(ACT_PERIOD_MIN)
                                .arg(ACT_PERIOD_MAX);
        qCritical() << error_msg.toStdString().c_str();
        return std::make_shared<ActBadRequest>(error_msg);
      }
    }

    sum += time_slot.GetPeriod();
  }

  // The sum of all periods should less than 999,999,999 ns
  if (sum > ACT_PERIOD_MAX) {
    double double_sum = sum / 1000.0;
    double double_period = ACT_PERIOD_MAX / 1000.0;
    QString error_msg = QString("Project (%1) - Cycle Time %2 should be less than %3 µs")
                            .arg(project.GetProjectName())
                            .arg(QString::number(double_sum, 'f', 3))
                            .arg(QString::number(double_period, 'f', 3));

    // QString error_msg = QString("The sum of all periods(Cycle Time) should less than %1 µs")
    //                         .arg(QString::number(ACT_PERIOD_MAX / 1000, 'f', 3));
    // .arg(QString::number(ACT_PERIOD_MAX / 1000, 'f', 3));
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The sum of all periods should equal to admin cycle time
  if (sum != cycle_setting.GetAdminCycleTime().GetNumerator()) {
    double double_sum = sum / 1000.0;
    double double_period = cycle_setting.GetAdminCycleTime().GetNumerator() / 1000.0;
    QString error_msg = QString("%1 - Cycle Time %2 should be equal to admin cycle time %3")
                            .arg(project.GetProjectName().toStdString().c_str())
                            .arg(QString::number(double_sum, 'f', 3))
                            .arg(QString::number(double_period, 'f', 3));
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (best_effort_found == false) {
    QString error_msg = QString("%1 - There should be just one Best Effort time slot in the system");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  if (time_sync_found == false) {
    QString error_msg = QString("%1 - There should be just one Time Sync time slot in the system");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Update the cycle setting to project
  project.SetCycleSetting(cycle_setting);

  return act_status;
}

}  // namespace core
}  // namespace act
