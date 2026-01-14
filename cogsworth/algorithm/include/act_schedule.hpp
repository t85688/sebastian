/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SCHEDULE_H
#define ACT_SCHEDULE_H

#include "act_json.hpp"
#include "act_project.hpp"
#include "act_schedule_config.hpp"
#include "act_schedule_result.hpp"
#include "act_schedule_stream.hpp"
#include "act_utilities.hpp"

class ActSchedule : public ActScheduleStream {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActScheduleConfig, schedule_config, ScheduleConfig);
  ACT_JSON_OBJECT(ActScheduleResult, schedule_result, ScheduleResult);

 public:
  ActSchedule() {}

  ACT_STATUS ComputeSchedule(ActProject &act_project);
  ACT_STATUS PrepareScheduleConfig(ActProject &act_project);
  ACT_STATUS GenerateScheduleResult(ActProject &act_project);
  ACT_STATUS AnalyzeScheduleResult(ActProject &act_project);
};

#endif