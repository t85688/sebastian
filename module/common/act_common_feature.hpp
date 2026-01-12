/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QDebug>
#include <QQueue>
#include <QString>
#include <thread>

#include "act_device.hpp"
#include "act_json.hpp"
// #include "act_southbound.hpp"
#include "act_status.hpp"

class ActCommonFeature {
  Q_GADGET

  ACT_JSON_FIELD(QString, feature_name, FeatureName);  ///< FeatureName item
  ACT_JSON_FIELD(bool, stop_flag, StopFlag);           ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);          ///< Progress item
                                                       ///< CommonFeature thread status

  // ActSouthbound southbound_;

 private:
 protected:
  ACT_STATUS feature_act_status_;
  // for thread
  std::unique_ptr<std::thread> feature_thread_;  ///< CommonFeature thread item
  /**
   * @brief Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress) {
    ACT_STATUS_INIT();
    this->progress_ = progress;
    qDebug() << __func__
             << QString("%1 Progress: %2 %.").arg(this->feature_name_).arg(GetProgress()).toStdString().c_str();
    return act_status;
  }

 public:
  // ACT_STATUS feature_act_status_;
  // // for thread
  // std::unique_ptr<std::thread> feature_thread_;  ///< CommonFeature thread item

  /**
   * @brief Construct a new Act Common Feature object
   *
   */
  ActCommonFeature()
      : progress_(0),
        stop_flag_(false),
        feature_act_status_(std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug)),
        feature_thread_(nullptr) {
    // qDebug() << __func__ << "feature_name_:" << feature_name_;
  }

  /**
   * @brief Destroy the Act Common Feature object
   *
   */
  ~ActCommonFeature() {
    if ((this->feature_thread_ != nullptr) && (this->feature_thread_->joinable())) {
      this->feature_thread_->join();
    }
    // qDebug() << __func__ << "feature_name_:" << feature_name_;
  }

  /**
   * @brief Stop thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop() {
    // Checking has the thread is running
    if (IsActStatusRunning(this->feature_act_status_)) {
      qDebug() << __func__ << QString("Stop %1 running thread.").arg(this->feature_name_).toStdString().c_str();

      // Send the stop signal and wait for the thread to finish.
      this->stop_flag_ = true;
      if ((this->feature_thread_ != nullptr) && (this->feature_thread_->joinable())) {
        this->feature_thread_->join();  // wait thread finished
      }
    } else {
      qDebug() << __func__ << QString("The %1 thread not running.").arg(this->feature_name_).toStdString().c_str();
    }

    return std::make_shared<ActProgressStatus>(ActStatusBase(*this->feature_act_status_), this->progress_);
  }

  /**
   * @brief Get the status of the thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus() {
    if (IsActStatusSuccess(this->feature_act_status_) && (this->progress_ == 100)) {
      this->feature_act_status_->SetStatus(ActStatusType::kFinished);
    }

    if ((!IsActStatusRunning(this->feature_act_status_)) && (!IsActStatusFinished(this->feature_act_status_))) {
      // failed
      return this->feature_act_status_;
    }

    return std::make_shared<ActProgressStatus>(ActStatusBase(*this->feature_act_status_), this->progress_);
  }
};
