#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include "act_intelligent.hpp"

act::intelligent::ActIntelligent::ActIntelligent() {
  auto objects_created =
      oatpp::base::Environment::getObjectsCreated();  // get count of objects created for a whole system lifetime

  // qDebug() << __func__ << "ObjectsCreated:" << objects_created;

  if (objects_created == 0) {
    qDebug() << __func__ << "oatpp::base::Environment::init()";
    oatpp::base::Environment::init();
  }

  // Assign the default value for thread
  progress_ = 0;
  stop_flag_ = false;
  intelligent_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);
  intelligent_thread_ = nullptr;
}

act::intelligent::ActIntelligent::~ActIntelligent() {
  if ((intelligent_thread_ != nullptr) && (intelligent_thread_->joinable())) {
    intelligent_thread_->join();
  }
}

ACT_STATUS act::intelligent::ActIntelligent::GetStatus() {
  if (IsActStatusSuccess(intelligent_act_status_) && (progress_ == 100)) {
    intelligent_act_status_->SetStatus(ActStatusType::kFinished);
  }

  if ((!IsActStatusRunning(intelligent_act_status_)) && (!IsActStatusFinished(intelligent_act_status_))) {
    // failed
    return intelligent_act_status_;
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*intelligent_act_status_), progress_);
}

ACT_STATUS act::intelligent::ActIntelligent::UpdateProgress(quint8 progress) {
  ACT_STATUS_INIT();
  progress_ = progress;
  qDebug() << __func__ << QString("Progress: %1%.").arg(GetProgress()).toStdString().c_str();
  return act_status;
}

ACT_STATUS act::intelligent::ActIntelligent::Stop() {
  // Checking has the thread is running
  if (IsActStatusRunning(intelligent_act_status_)) {
    qDebug() << "Stop Intelligent's running thread.";
    // Send the stop signal and wait for the thread to finish.
    stop_flag_ = true;

    if ((intelligent_thread_ != nullptr) && (intelligent_thread_->joinable())) {
      intelligent_thread_->join();  // wait thread finished
    }
    // TODO: send stop request to AI server
  } else {
    qDebug() << __func__ << "The Intelligent's thread not running.";
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*intelligent_act_status_), progress_);
}

ACT_STATUS act::intelligent::ActIntelligent::StartIntelligentRequest(
    const ActIntelligentRecognizeRequest &recognize_request,
    const ActAIServerConnectConfiguration &server_connect_cfg) {
  ACT_STATUS_INIT();

  // Checking has the thread is running
  if (IsActStatusRunning(intelligent_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  // init ActTopologyMapping status
  progress_ = 0;
  stop_flag_ = false;
  intelligent_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((intelligent_thread_ != nullptr) && (intelligent_thread_->joinable())) {
      intelligent_thread_->join();
    }
    intelligent_act_status_->SetStatus(ActStatusType::kRunning);
    intelligent_thread_ =
        std::make_unique<std::thread>(&act::intelligent::ActIntelligent::TriggerIntelligentRequestSenderForThread, this,
                                      std::cref(recognize_request), std::cref(server_connect_cfg));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerIntelligentRequestSenderForThread";
    HRESULT hr = SetThreadDescription(intelligent_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start IntelligentRequestSender thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(IntelligentRequestSender) failed. Error:" << e.what();
    intelligent_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*intelligent_act_status_), progress_);
}

void act::intelligent::ActIntelligent::TriggerIntelligentRequestSenderForThread(
    const ActIntelligentRecognizeRequest &recognize_request,
    const ActAIServerConnectConfiguration &server_connect_cfg) {
  // Triggered the Mapper and wait for the return, and update intelligent_act_status_.
  try {
    intelligent_act_status_ = IntelligentRequestSender(recognize_request, server_connect_cfg);

  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    intelligent_act_status_ = std::make_shared<ActStatusInternalError>("Intelligent");
  }
}

ACT_STATUS act::intelligent::ActIntelligent::IntelligentRequestSender(
    const ActIntelligentRecognizeRequest &recognize_request,
    const ActAIServerConnectConfiguration &server_connect_cfg) {
  ACT_STATUS_INIT();

  ActMoxaAIClientAgent client_agent(server_connect_cfg.GetBaseurl(), server_connect_cfg.GetProxy());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.PostTheRecognizeRequestAndUseStreamingReceive(recognize_request, response_queue_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostTheRecognizeRequestAndUseStreamingReceive() failed.";
    return act_status;
  }

  UpdateProgress(100);
  return act_status;
}

ACT_STATUS act::intelligent::ActIntelligent::StartIntelligentQuestionnaireUpload(
    const ActIntelligentQuestionnaireUpload &upload, const ActAIServerConnectConfiguration &server_connect_cfg) {
  ACT_STATUS_INIT();

  // Checking has the thread is running
  if (IsActStatusRunning(intelligent_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  // init ActTopologyMapping status
  progress_ = 0;
  stop_flag_ = false;
  intelligent_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((intelligent_thread_ != nullptr) && (intelligent_thread_->joinable())) {
      intelligent_thread_->join();
    }
    intelligent_act_status_->SetStatus(ActStatusType::kRunning);
    intelligent_thread_ =
        std::make_unique<std::thread>(&act::intelligent::ActIntelligent::TriggerIntelligentQuestionnaireUploadThread,
                                      this, std::cref(upload), std::cref(server_connect_cfg));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerIntelligentQuestionnaireUploadThread";
    HRESULT hr = SetThreadDescription(intelligent_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start IntelligentQuestionnaireUpload thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(IntelligentQuestionnaireUpload) failed. Error:" << e.what();
    intelligent_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*intelligent_act_status_), progress_);
}

void act::intelligent::ActIntelligent::TriggerIntelligentQuestionnaireUploadThread(
    const ActIntelligentQuestionnaireUpload &upload, const ActAIServerConnectConfiguration &server_connect_cfg) {
  // Triggered the Mapper and wait for the return, and update intelligent_act_status_.
  try {
    intelligent_act_status_ = IntelligentQuestionnaireUpload(upload, server_connect_cfg);

  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    intelligent_act_status_ = std::make_shared<ActStatusInternalError>("Intelligent");
  }
}

ACT_STATUS act::intelligent::ActIntelligent::IntelligentQuestionnaireUpload(
    const ActIntelligentQuestionnaireUpload &upload, const ActAIServerConnectConfiguration &server_connect_cfg) {
  ACT_STATUS_INIT();

  ActMoxaAIClientAgent client_agent(server_connect_cfg.GetBaseurl(), server_connect_cfg.GetProxy());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.PostTheQuestionnaireUpload(upload, response_queue_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostTheQuestionnaireUpload() failed.";
    return act_status;
  }

  UpdateProgress(100);
  return act_status;
}

ACT_STATUS act::intelligent::ActIntelligent::StartIntelligentQuestionnaireDownload(
    const ActIntelligentQuestionnaireDownload &download, const ActAIServerConnectConfiguration &server_connect_cfg) {
  ACT_STATUS_INIT();

  // Checking has the thread is running
  if (IsActStatusRunning(intelligent_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  // init ActTopologyMapping status
  progress_ = 0;
  stop_flag_ = false;
  intelligent_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((intelligent_thread_ != nullptr) && (intelligent_thread_->joinable())) {
      intelligent_thread_->join();
    }
    intelligent_act_status_->SetStatus(ActStatusType::kRunning);
    intelligent_thread_ =
        std::make_unique<std::thread>(&act::intelligent::ActIntelligent::TriggerIntelligentQuestionnaireDownloadThread,
                                      this, std::cref(download), std::cref(server_connect_cfg));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerIntelligentQuestionnaireDownloadThread";
    HRESULT hr = SetThreadDescription(intelligent_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start IntelligentQuestionnaireDownload thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(IntelligentQuestionnaireDownload) failed. Error:" << e.what();
    intelligent_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*intelligent_act_status_), progress_);
}

void act::intelligent::ActIntelligent::TriggerIntelligentQuestionnaireDownloadThread(
    const ActIntelligentQuestionnaireDownload &download, const ActAIServerConnectConfiguration &server_connect_cfg) {
  // Triggered the Mapper and wait for the return, and update intelligent_act_status_.
  try {
    intelligent_act_status_ = IntelligentQuestionnaireDownloadSender(download, server_connect_cfg);

  } catch (std::exception &e) {
    // no effect on the program's behavior
    static_cast<void>(e);
    intelligent_act_status_ = std::make_shared<ActStatusInternalError>("Intelligent");
  }
}

ACT_STATUS act::intelligent::ActIntelligent::IntelligentQuestionnaireDownloadSender(
    const ActIntelligentQuestionnaireDownload &download, const ActAIServerConnectConfiguration &server_connect_cfg) {
  ACT_STATUS_INIT();

  ActMoxaAIClientAgent client_agent(server_connect_cfg.GetBaseurl(), server_connect_cfg.GetProxy());

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Get Questionnaire Template
  act_status = client_agent.GetQuestionnaireTemplate(download, response_queue_);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "GetQuestionnaireTemplate() failed.";
    return act_status;
  }

  UpdateProgress(100);
  return act_status;
}

ACT_STATUS act::intelligent::ActIntelligent::SendRequestToMoxaAIServer(
    const ActIntelligentRecognizeRequest &recognize_request, const QString &base_url, const QString &proxy,
    ActIntelligentResponse &response) {
  ACT_STATUS_INIT();

  ActMoxaAIClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.PostTheRecognizeRequest(recognize_request, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostTheRecognizeRequest() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::intelligent::ActIntelligent::SendReportToMoxaAIServer(const ActIntelligentReport &report,
                                                                      const QString &base_url, const QString &proxy,
                                                                      ActIntelligentResponse &response) {
  ACT_STATUS_INIT();

  ActMoxaAIClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.PostTheRecognizeReport(report, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostTheRecognizeReport() failed.";
    return act_status;
  }

  return act_status;
}

ACT_STATUS act::intelligent::ActIntelligent::GetHistoryFromMoxaAIServer(const ActIntelligentHistory &report,
                                                                        const QString &base_url, const QString &proxy,
                                                                        ActIntelligentHistoryResponse &response) {
  ACT_STATUS_INIT();

  ActMoxaAIClientAgent client_agent(base_url, proxy);

  // Init
  client_agent.Init();
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "Init() failed.";
    return act_status;
  }

  // Post Recognize
  act_status = client_agent.GetHistory(report, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << __func__ << "PostTheRecognizeReport() failed.";
    return act_status;
  }

  return act_status;
}
