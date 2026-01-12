
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

// #include <QRandomGenerator>

#include "act_core.hpp"

namespace act {
namespace core {  // namespace core

void ActCore::StartChamberlainThread() {
  ACT_STATUS_INIT();

  // qint8 num_workers = static_cast<qint8>(workers_.size());

  quint64 monitor_count = 0;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (g_act_process_status == ActProcessStatus::Running) {
    act::core::g_core.CheckTokenHardTimeout();

    act::core::g_core.CheckServerProjectExpired();

    // Avoid the detection too often
    if (monitor_count > 5) {
      monitor_count = 0;
    }

    monitor_count++;
    SLEEP_MS(1000);
  }

  return;
}

ACT_STATUS ActCore::StartChamberlain() {
  ACT_STATUS_INIT();
  this->chamberlain_thread_ = std::make_shared<std::thread>(&act::core::ActCore::StartChamberlainThread, this);

#ifdef _WIN32
  // Set the thread name
  std::wstring thread_name = L"Chamberlain";
  HRESULT hr = SetThreadDescription(this->chamberlain_thread_->native_handle(), thread_name.c_str());
  if (FAILED(hr)) {
    // Handle error
  }
#endif

  return act_status;
}

void ActCore::StopChamberlain() {
  qDebug() << "Ready to stop Chamberlain thread";

  if ((this->chamberlain_thread_ != nullptr) && (this->chamberlain_thread_->joinable())) {
    this->chamberlain_thread_->join();
  }

  qDebug() << "Chamberlain thread is finish";

  return;
}

}  // namespace core
}  // namespace act