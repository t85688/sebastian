#include "act_schedule.hpp"

ACT_STATUS ActSchedule::PrepareScheduleConfig(ActProject &act_project) {
  ACT_STATUS_INIT();

  act_status = schedule_config_.PrepareScheduleConfigVLAN(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = schedule_config_.PrepareScheduleConfigDevice(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = schedule_config_.PrepareScheduleConfigLink(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = schedule_config_.PrepareScheduleConfigStream(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  schedule_config_.ComputeCycleTime();

  return act_status;
}

ACT_STATUS ActSchedule::ComputeSchedule(ActProject &act_project) {
  ACT_STATUS_INIT();

  QList<qint64> stream_list = schedule_config_.SortScheduleConfigStreams();

  for (qint64 &stream_id : stream_list) {
    ActScheduleConfigStream &schedule_config_stream = schedule_config_.GetScheduleConfigStream(stream_id);
    QList<quint8> pcps = schedule_config_stream.GetPCPs().values();
    std::sort(pcps.begin(), pcps.end(), std::greater<quint8>());

    for (quint8 pcp : pcps) {
      quint8 queue_id = pcp;
      ActScheduleResult schedule_result = this->schedule_result_;
      ActScheduleStream::SetScheduleStream(stream_id, queue_id, pcp, schedule_config_stream.GetEarliestTransmitOffset(),
                                           schedule_config_stream.GetInterval(), schedule_config_.GetCycleTime());

      act_status = ActScheduleStream::ComputeScheduleStream(schedule_config_, schedule_result);
      if (IsActStatusSuccess(act_status)) {
        schedule_config_stream.SetPriorityCodePoint(pcp);
        schedule_config_stream.SetQueueId(queue_id);
        this->schedule_result_ = schedule_result;
        break;
      }
    }

    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActSchedule::GenerateScheduleResult(ActProject &act_project) {
  ACT_STATUS_INIT();

  ActComputedResult &computed_result = act_project.GetComputedResult();

  this->schedule_result_.GenerateRoutingResult(computed_result, this->schedule_config_);
  this->schedule_result_.GenerateDeviceResult(computed_result, this->schedule_config_);
  this->schedule_result_.GenerateStreamResult(computed_result, this->schedule_config_);
  this->schedule_result_.GenerateGCLResult(computed_result, this->schedule_config_);

  computed_result.SetDevices(act_project.GetDevices());
  computed_result.SetLinks(act_project.GetLinks());
  computed_result.SetTrafficDesign(act_project.GetTrafficDesign());

  return act_status;
}

ACT_STATUS ActSchedule::AnalyzeScheduleResult(ActProject &act_project) {
  ACT_STATUS_INIT();

  ActComputedResult &computed_result = act_project.GetComputedResult();
  QSet<ActRoutingResult> &routing_results = computed_result.GetRoutingResults();

  for (ActRoutingResult routing_result : routing_results) {
    act_status = ActScheduleFeasibility::CheckRoutingResultFeasibility(routing_result, act_project);
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
  }

  return ACT_STATUS_SUCCESS;
}