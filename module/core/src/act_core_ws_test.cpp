#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include <QSet>
#include <thread>

#include "act_core.hpp"

namespace act {
namespace core {

ActWSTest::~ActWSTest() {
  if ((thread_ != nullptr) && (thread_->joinable())) {
    thread_->join();
  }
}

ACT_STATUS ActWSTest::Start() {
  qDebug() << "Start!!!";
  stop_flag_ = false;
  progress_ = 0;
  thread_ = std::make_unique<std::thread>(&ActWSTest::Loop, this);
  act_status_->SetStatus(ActStatusType::kRunning);
  return std::make_shared<ActProgressStatus>(ActStatusBase(*act_status_), progress_);
}

ACT_STATUS ActWSTest::Stop() {
  qDebug() << "Stop!!!";
  stop_flag_ = true;
  if ((thread_ != nullptr) && (thread_->joinable())) {
    thread_->join();
  }
  act_status_->SetStatus(ActStatusType::kStop);
  return std::make_shared<ActProgressStatus>(ActStatusBase(*act_status_), progress_);
}

void ActWSTest::Loop() {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  while (!stop_flag_) {
    SLEEP_MS(1000);
    qDebug() << "running...";
    progress_ += 5;

    if (progress_ == 100) {
      break;
    }
  }
  stop_flag_ = true;
  act_status_->SetStatus(ActStatusType::kFinished);
  qDebug() << "Finish!!";
}

ACT_STATUS ActWSTest::GetStatus() {
  return std::make_shared<ActProgressStatus>(ActStatusBase(*act_status_), progress_);
}

void ActCore::StartWSTestThread(const qint64 &ws_listener_id, std::future<void> signal_receiver, qint64 project_id) {
  ACT_STATUS_INIT();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Get project failed with project id:" << project_id;

    // ACT_STATUS act_status = std::make_shared<ActStatusBase>()
    // #define ACT_STATUS std::shared_ptr<ActStatusBase>
    // #define ACT_STATUS_INIT() ACT_STATUS act_status = std::make_shared<ActStatusBase>()

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp;
    ws_resp = ActWSResponseErrorTransfer(ActWSCommandEnum::kTestStart, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    return;
  }

  ActWSTest ws_test;
  act_status = ws_test.Start();
  if (!IsActStatusRunning(act_status)) {
    qCritical() << "Start WS Test failed";

    ActProgressWSResponse ws_resp(ActWSCommandEnum::kTestStart, dynamic_cast<ActProgressStatus &>(*act_status));
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kComputing;  // for test

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
    qDebug() << "Get Status";
    act_status = ws_test.GetStatus();

    if (act_status->GetStatus() != ActStatusType::kRunning) {  // error

      std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
          ActWSResponseErrorTransfer(ActWSCommandEnum::kTestStart, *act_status);
      qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

    } else {
      ActProgressWSResponse ws_resp(ActWSCommandEnum::kTestStart, dynamic_cast<ActProgressStatus &>(*act_status));
      qDebug() << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      break;
    }
  }
  // std::this_thread::sleep_for(std::chrono::seconds(10));

  qDebug() << "Thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    ws_test.Stop();
    qCritical() << project.GetProjectName() << "Abort ws test";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;
    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;

  return;
}

ACT_STATUS ActCore::StartWSTest(qint64 &project_id, const qint64 &ws_listener_id) {
  ACT_STATUS_INIT();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kComputing, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartCompute, *act_status);
    this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << "Spawn test thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartWSTestThread, this, std::cref(ws_listener_id),
                                             std::move(signal_receiver), project_id);

  // Insert thread handler to pools
  qDebug() << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  // Wait for 10 sec
  // std::this_thread::sleep_for(std::chrono::seconds(10));
  // qDebug() << "Asking Thread to Stop";

  // Set the value in promise
  // signal_sender->set_value();
  // thread_ptr->join();

  return act_status;
}

ACT_STATUS ActCore::StopWSTest(qint64 &project_id, const qint64 &ws_listener_id) {
  return act::core::g_core.StopWSJob(project_id, ws_listener_id, ActWSCommandEnum::kTestStop);
  // ACT_STATUS_INIT();

  // qDebug() << "Asking Thread to Stop";
  // if (!ws_thread_handler_pools.contains(project_id)) {
  //   qDebug() << "Thread not exist";
  //   ActBadRequest act_bad_status("Thread not exist");
  //   this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, act_bad_status, ws_listener_id);
  //   // socket.sendClose();
  //   return act_status;
  // }

  // // Fetch the promise instance of the thread
  // // std::shared_ptr<std::promise<void>> signal_sender = ws_thread_handler_pools.value(project_id);
  // std::pair<std::shared_ptr<std::promise<void>>, std::shared_ptr<std::thread>> thread_handler =
  //     ws_thread_handler_pools.value(project_id);
  // std::shared_ptr<std::promise<void>> signal_sender = thread_handler.first;
  // std::shared_ptr<std::thread> thread_ptr = thread_handler.second;

  // // Set the value in promise to stop the thread
  // if (signal_sender) {
  //   signal_sender->set_value();
  // }
  // qDebug() << "Waiting for the thread close";
  // // Waiting for the thread close
  // if (thread_ptr) {
  //   thread_ptr->join();
  // }
  // qDebug() << "The thread is close";

  // return act_status;
}

}  // namespace core
}  // namespace act
