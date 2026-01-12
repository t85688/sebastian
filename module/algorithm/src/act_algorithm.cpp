#include "act_algorithm.hpp"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

// #include "act_algorithm_flow.h"
// #include "act_core.hpp"

ActAlgorithm::~ActAlgorithm() {
  if ((computing_thread_ != nullptr) && (computing_thread_->joinable())) {
    computing_thread_->join();
  }
}

ACT_STATUS ActAlgorithm::Start(ActProject &act_project) {
  // Checking has the thread is running
  if (IsActStatusRunning(compute_act_status_)) {
    qCritical() << "In project" << this->GetProjectName() << "currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Scheduling");
  }

  qDebug() << "In project" << this->GetProjectName() << "compute thread is going to start.";

  // Initial compute status
  progress_ = 0;
  stop_computing_flag_ = false;
  compute_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to triggered the compute
  try {
    // check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((computing_thread_ != nullptr) && (computing_thread_->joinable())) {
      computing_thread_->join();
    }
    compute_act_status_->SetStatus(ActStatusType::kRunning);
    computing_thread_ = std::make_unique<std::thread>(&ActAlgorithm::ComputeLoop, this, std::ref(act_project));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"ComputingThread";
    HRESULT hr = SetThreadDescription(computing_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(compute) failed. Error:" << e.what();
    compute_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Scheduling");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*compute_act_status_), progress_);
}

ACT_STATUS ActAlgorithm::GetStatus() {
  if (IsActStatusSuccess(compute_act_status_) && (progress_ == 100)) {
    compute_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(compute_act_status_)) && (!IsActStatusFinished(compute_act_status_))) {
    // failed
    return compute_act_status_;
  }

  // Let the progress bar jump!
  progress_count_++;
  if (progress_count_ > 2) {
    if (progress_ >= 35 && progress_ < 99) {
      progress_++;
    }
    progress_count_ = 0;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*compute_act_status_), progress_);
}

ACT_STATUS ActAlgorithm::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(compute_act_status_)) {
    qDebug() << "In project" << this->GetProjectName() << "compute thread is going to stop.";

    // Send the stop signal to the computer and wait for the thread to finish.
    stop_computing_flag_ = true;
    // wait thread finished
    if ((computing_thread_ != nullptr) && (computing_thread_->joinable())) {
      computing_thread_->join();
    }
    compute_act_status_->SetStatus(ActStatusType::kStop);
  } else {
    qDebug() << "In project" << this->GetProjectName() << "there is no compute thread is running.";
  }
  return std::make_shared<ActProgressStatus>(ActStatusBase(*compute_act_status_), progress_);
}

void ActAlgorithm::ComputeLoop(ActProject &act_project) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  qDebug() << "In project" << this->GetProjectName() << "calling algorithm module.";

  compute_act_status_ = this->DoAdvancedSchedule(act_project);

  if (!IsActStatusSuccess(compute_act_status_)) {
    return;
  }

  compute_act_status_->SetStatus(ActStatusType::kFinished);
  compute_act_status_->SetSeverity(ActSeverity::kInformation);
  qDebug() << "In project" << this->GetProjectName() << "the compute procedure is finished";
  return;
}

ACT_STATUS ActAlgorithm::DoAdvancedSchedule(ActProject &act_project) {
  ACT_STATUS_INIT();

  ActSchedule schedule;
  act_status = schedule.PrepareScheduleConfig(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = schedule.ComputeSchedule(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = schedule.GenerateScheduleResult(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  act_status = schedule.AnalyzeScheduleResult(act_project);
  if (!IsActStatusSuccess(act_status)) {
    return act_status;
  }

  return act_status;
}