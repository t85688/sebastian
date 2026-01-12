/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_SCHEDULE_FEASIBILITY_H
#define ACT_SCHEDULE_FEASIBILITY_H

#include "act_json.hpp"
#include "act_schedule_config.hpp"
#include "act_schedule_result.hpp"
#include "act_utilities.hpp"

class ActScheduleFeasibility : public QSerializer {
 public:
  ActScheduleFeasibility() {}

  ACT_STATUS CheckTransmitOffset(ActScheduleConfigStream &schedule_config_stream, qint64 &transmit_offset,
                                 qint64 &interval, qint64 &sequence_id);

  ACT_STATUS CheckReceiveOffset(ActScheduleResultFrame &schedule_result_frame, ActScheduleConfig &schedule_config);

  ACT_STATUS CheckRoutingPathDuplicated(ActScheduleResultFrame &schedule_result_frame,
                                        ActScheduleConfig &schedule_config);

  ACT_STATUS CheckDeviceFeature(ActScheduleResultFrame &schedule_result_frame, ActScheduleConfig &schedule_config);

  ACT_STATUS CheckGateControlListLength(ActScheduleResultFrame &schedule_result_frame,
                                        ActScheduleResult &schedule_result, ActScheduleConfig &schedule_config);

  ACT_STATUS CheckQueueSize(ActScheduleResultFrame &schedule_result_frame, ActScheduleResult &schedule_result,
                            ActScheduleConfig &schedule_config);

  ACT_STATUS CheckRoutingResultFeasibility(ActRoutingResult &routing_result, ActProject &act_project);
};

#endif