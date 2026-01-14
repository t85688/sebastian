/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_ALGORITHM_H
#define ACT_ALGORITHM_H

#include <QRandomGenerator>
#include <future>  // for std::promise, std::future
#include <thread>

#include "act_json.hpp"
#include "act_project.hpp"
#include "act_schedule.hpp"
#include "act_status.hpp"
#include "act_utilities.hpp"
#include "gcl/act_gcl_result.hpp"
#include "routing/act_routing_result.hpp"

#define LISTENER_NO_SCHEDULE (-2)
#define ACT_RESERVE_BANDWIDTH_100M (125000)  ///< reserve bandwidth for 100M (125 us)
#define ACT_RESERVE_BANDWIDTH_1G (12500)     ///< reserve bandwidth for 100M (12.5 us)

#define REMAIN_STREAM_ID (0)
#define BEST_EFFORT_STREAM_ID (-1)
#define TIME_SYNC_STREAM_ID (-2)

#define GCL_RESERVE (50)

class ActAlgorithm {
  Q_GADGET

  ACT_JSON_FIELD(bool, stop_computing_flag, StopComputingFlag);  ///< The flag to stop computing
  ACT_JSON_FIELD(quint8, progress, Progress);                    ///< Progress item
  ACT_JSON_FIELD(QString, project_name, ProjectName);            ///< The name of the running project

  std::shared_ptr<ActStatusBase> act_status_;
  ACT_STATUS compute_act_status_;                  ///< Computer thread status
  std::shared_ptr<std::thread> computing_thread_;  ///< The thread for compute

 private:
  quint8 progress_count_;

 public:
  /**
   * @brief Construct a new Act Compute object
   *
   */
  ActAlgorithm()
      : progress_(0),
        progress_count_(0),
        stop_computing_flag_(false),
        compute_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kFinished, ActSeverity::kDebug)),
        computing_thread_(nullptr) {}

  ActAlgorithm(QString project_name)
      : progress_(0),
        progress_count_(0),
        stop_computing_flag_(false),
        compute_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kFinished, ActSeverity::kDebug)),
        computing_thread_(nullptr),
        project_name_(project_name) {}

  /**
   * @brief Destroy the Act Compute object
   *
   */
  ~ActAlgorithm();

  /**
   * @brief Compute routing and scheduling
   *
   * @param act_project
   * @return ACT_STATUS
   */
  ACT_STATUS Start(ActProject &act_project);

  /**
   * @brief Ask algorithm thread to stop
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the computing thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief compute internal thread callback function
   *
   * @param act_project
   */
  void ComputeLoop(ActProject &act_project);

  ACT_STATUS DoAdvancedSchedule(ActProject &act_project);
};

#endif /* ACT_ALGORITHM_H */