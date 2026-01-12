#include <QHostInfo>
#include <QNetworkInterface>
#include <QSysInfo>
#include <QUrl>

#include "act_core.hpp"
#include "act_intelligent.hpp"
#include "act_system.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::IntelligentRequest(ActIntelligentRecognizeRequest &request, ActIntelligentResponse &response) {
  ACT_STATUS_INIT();

  // Check license
  /* if (!this->GetLicense().GetFeature().GetIntelligent()) {
    QString error_msg = QString("The License not support Intelligent.");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } */

  // Get AI server URL(domain & port)
  ActSystem sys = this->GetSystemConfig();
  QString ai_assistant_local_endpoint = sys.GetIntelligentLocalEndpoint();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  request.SetIntelligentEndpoint(sys.GetIntelligentEndpoint());
  request.SetSessionId(sys.GetSerialNumber());
  request.SetSysVersion(sys.GetActVersion());

  // Send user input to Moxa AI server
  // qDebug() << __func__
  //          << QString("Use the %1 to connect the AI Server")
  //                 .arg(kActRestfulProtocolEnumMap.key(protocol))
  //                 .toStdString()
  //                 .c_str();

  act::intelligent::ActIntelligent intelligent;
  act_status =
      intelligent.SendRequestToMoxaAIServer(request, ai_assistant_local_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendRequestToMoxaAIServer() failed with project id:" << request.GetProjectId();
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::IntelligentReport(ActIntelligentReport &report, ActIntelligentResponse &response) {
  ACT_STATUS_INIT();

  // Check license
  /* if (!this->GetLicense().GetFeature().GetIntelligent()) {
    QString error_msg = QString("The License not support Intelligent.");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } */

  // Get AI server URL(domain & port)
  ActSystem sys = this->GetSystemConfig();
  QString ai_assistant_local_endpoint = sys.GetIntelligentLocalEndpoint();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();
  qDebug() << __func__ << QString("AI server endpoint: %1").arg(ai_assistant_local_endpoint).toStdString().c_str();

  // Set IntelligentEndpoint to Report request
  report.SetIntelligentEndpoint(sys.GetIntelligentEndpoint());

  act::intelligent::ActIntelligent intelligent;
  act_status = intelligent.SendReportToMoxaAIServer(report, ai_assistant_local_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "SendReportToMoxaAIServer() failed";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::GetIntelligentHistory(ActIntelligentHistory &history, ActIntelligentHistoryResponse &response) {
  ACT_STATUS_INIT();

  // Check license
  /* if (!this->GetLicense().GetFeature().GetIntelligent()) {
    QString error_msg = QString("The License not support Intelligent.");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  } */

  // Get AI server URL(domain & port)
  ActSystem sys = this->GetSystemConfig();
  QString ai_assistant_local_endpoint = sys.GetIntelligentLocalEndpoint();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  // Set IntelligentEndpoint to Post Recognize request
  history.SetIntelligentEndpoint(sys.GetIntelligentEndpoint());

  // Set SetSessionId to Post Recognize request
  history.SetSessionId(sys.GetSerialNumber());

  // Set sysVersion to Post Recognize request
  history.SetSysVersion(sys.GetActVersion());

  // TODO: Temporary backend handling, remove once frontend is implemented
  ActProject project;
  qint64 project_id = history.GetProjectId().toInt();
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }
  history.SetProfile(project.GetProfile());
  history.SetProjectMode(project.GetProjectMode());
  history.SetQuestionnaireMode(project.GetQuestionnaireMode());

  act::intelligent::ActIntelligent intelligent;
  act_status =
      intelligent.GetHistoryFromMoxaAIServer(history, ai_assistant_local_endpoint, http_proxy_endpoint, response);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "GetHistoryFromMoxaAIServer() failed with project id:" << history.GetProjectId();
    return act_status;
  }

  return act_status;
}

// From Websocket request

void ActCore::StartIntelligentRequestThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                            qint64 project_id, ActIntelligentRecognizeRequest request) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentRequest, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  }

  // Check license
  /* if (!this->GetLicense().GetFeature().GetIntelligent()) {
    QString error_msg = QString("The License not support Intelligent.");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentRequest, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  } */

  // Get AI server URL(domain & port)
  ActSystem sys = this->GetSystemConfig();
  QString ai_assistant_local_endpoint = sys.GetIntelligentLocalEndpoint();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  request.SetProjectId(QString::number(project_id));

  request.SetIntelligentEndpoint(this->GetSystemConfig().GetIntelligentEndpoint());

  request.SetSessionId(sys.GetSerialNumber());

  request.SetSysVersion(sys.GetActVersion());

  // TODO: Temporary backend handling, remove once frontend is implemented
  request.SetProfile(project.GetProfile());
  request.SetProjectMode(project.GetProjectMode());
  request.SetQuestionnaireMode(project.GetQuestionnaireMode());

  // Start MultipleTransaction
  act::core::g_core.EnableMultipleTransaction(project_id);

  // Start send user input to Moxa AI server
  act::intelligent::ActIntelligent intelligent;
  act::intelligent::ActAIServerConnectConfiguration server_connect_cfg(ai_assistant_local_endpoint,
                                                                       http_proxy_endpoint);
  act_status = intelligent.StartIntelligentRequest(request, server_connect_cfg);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start Intelligent request failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentRequest, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

    // Disable MultipleTransaction & Stop Transaction
    act::core::g_core.DisableMultipleTransaction(project_id);
    act::core::g_core.StopTransaction(project_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kIntelligentRequestSending;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout) {
    act_status = intelligent.GetStatus();

    // Dequeue
    while (!intelligent.response_queue_.isEmpty()) {
      ActIntelligentResponse intelligent_response = intelligent.response_queue_.dequeue();
      ActIntelligentResponseWSResponse ws_resp(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kRunning,
                                               intelligent_response);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentRequest, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qInfo() << project.GetProjectName() << "Thread is going to close";

  // Disable MultipleTransaction
  act::core::g_core.DisableMultipleTransaction(project_id);

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    intelligent.Stop();
    qCritical() << project.GetProjectName() << "Abort Intelligent request";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    // Stop Transaction
    act::core::g_core.StopTransaction(project_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  // Commit Transaction
  act::core::g_core.CommitTransaction(project_id);

  act_status = this->GetProject(project_id, project);
  if (IsActStatusSuccess(act_status)) {
    // Send update msg after all intelligent requests are done
    ActProjectPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project, true);
    this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project.GetId());
  }

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartIntelligentRequest, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartIntelligentRequest(qint64 &project_id, const qint64 &ws_listener_id,
                                            ActIntelligentRecognizeRequest &request) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kIntelligentRequestSending, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentRequest, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn intelligent request thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartIntelligentRequestThread, this, std::cref(ws_listener_id),
                                    std::move(signal_receiver), project_id, request);

  // #ifdef _WIN32
  //   // Set the thread name
  //   std::wstring thread_name = L"StartIntelligentRequestThread";
  //   HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  //   if (FAILED(hr)) {
  //     // Handle error
  //   }
  // #endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

ACT_STATUS ActCore::IntelligentUploadFile(ActIntelligentUploadFile &upload_file) {
  ACT_STATUS_INIT();

  // Check if the directory exists, create it if not
  QDir dir(GetIntelligentFilePath());
  if (!dir.exists()) {
    if (!QDir().mkpath(GetIntelligentFilePath())) {
      qDebug() << "mkpath() failed:" << GetIntelligentFilePath();
      return std::make_shared<ActStatusInternalError>("Database");
    }
  }

  QFile file(QString("%1/%2").arg(GetIntelligentFilePath()).arg(upload_file.GetFileName()));
  if (!file.open(QIODevice::WriteOnly)) {
    qDebug() << "Open intelligent file failed:" << file.fileName();
    return std::make_shared<ActStatusInternalError>("Intelligent");
  }

  file.write(upload_file.GetFile().toStdString().c_str());
  file.close();

  return act_status;
}

ACT_STATUS ActCore::StartIntelligentQuestionnaireUpload(qint64 &project_id, const qint64 &ws_listener_id,
                                                        ActIntelligentQuestionnaireUpload &upload) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check job can start
  QString project_name;
  if (!upload.GetVerify()) {
    project_id = ACT_SYSTEM_WS_PROJECT_ID;
    project_name = ACT_SYSTEM_WS_PROJECT_NAME;
  }

  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kIntelligentUploadSending, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << "Spawn intelligent questionnaire upload thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr = std::make_shared<std::thread>(&act::core::ActCore::StartIntelligentQuestionnaireUploadThread, this,
                                             std::cref(ws_listener_id), std::move(signal_receiver), project_id, upload);

  // #ifdef _WIN32
  //   // Set the thread name
  //   std::wstring thread_name = L"StartIntelligentRequestThread";
  //   HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  //   if (FAILED(hr)) {
  //     // Handle error
  //   }
  // #endif

  // Insert thread handler to pools
  qDebug() << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartIntelligentQuestionnaireUploadThread(const qint64 &ws_listener_id, std::future<void> signal_receiver,
                                                        qint64 project_id, ActIntelligentQuestionnaireUpload upload) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  if (upload.GetVerify()) {
    // Get project by id
    ActProject project;
    act_status = this->GetProject(project_id, project);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get project failed with project id:" << project_id;

      std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
          ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, *act_status);
      this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
      return;
    }

    // TODO: Temporary backend handling, remove once frontend is implemented
    upload.SetProfile(project.GetProfile());
    upload.SetProjectMode(project.GetProjectMode());
    upload.SetQuestionnaireMode(project.GetQuestionnaireMode());
  }

  // Check license
  /* if (!this->GetLicense().GetFeature().GetIntelligent()) {
    QString error_msg = QString("The License not support Intelligent.");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  } */

  // Get AI server URL(domain & port)
  ActSystem sys = this->GetSystemConfig();
  QString ai_assistant_local_endpoint = sys.GetIntelligentLocalEndpoint();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  upload.SetProjectId(QString::number(project_id));

  upload.SetIntelligentEndpoint(this->GetSystemConfig().GetIntelligentEndpoint());

  upload.SetSessionId(sys.GetSerialNumber());

  upload.SetSysVersion(sys.GetActVersion());

  QFile file(QString("%1/%2").arg(GetIntelligentFilePath()).arg(upload.GetFileName()));
  if (!file.open(QIODevice::ReadOnly)) {
    QString error_msg = QString("Open intelligent file %1 failed").arg(file.fileName());
    qCritical() << __func__ << error_msg.toStdString().c_str();

    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  }

  upload.SetFile(QString::fromUtf8(file.readAll()));
  file.close();

  // Start send user input to Moxa AI server
  act::intelligent::ActIntelligent intelligent;
  act::intelligent::ActAIServerConnectConfiguration server_connect_cfg(ai_assistant_local_endpoint,
                                                                       http_proxy_endpoint);
  act_status = intelligent.StartIntelligentQuestionnaireUpload(upload, server_connect_cfg);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << "Start Intelligent questionnaire upload failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kIntelligentUploadSending;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout) {
    act_status = intelligent.GetStatus();

    // Dequeue
    while (!intelligent.response_queue_.isEmpty()) {
      ActIntelligentResponse intelligent_response = intelligent.response_queue_.dequeue();
      ActIntelligentResponseWSResponse ws_resp(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload,
                                               ActStatusType::kRunning, intelligent_response);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_resp, ws_listener_id);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qInfo() << "Upload questionnaire thread is going to close";

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    intelligent.Stop();
    qCritical() << "Abort Intelligent questionnaire upload";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    // // Stop Transaction
    // act::core::g_core.StopTransaction(project_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  // // Commit Transaction
  // act::core::g_core.CommitTransaction(project_id);

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartIntelligentQuestionnaireUpload, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSystem, false, ws_resp, ws_listener_id);

  return;
}

ACT_STATUS ActCore::StartIntelligentQuestionnaireDownload(qint64 &project_id, const qint64 &ws_listener_id,
                                                          ActIntelligentQuestionnaireDownload &download) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Check if the directory exists, create it if not
  QDir dir(GetIntelligentFilePath());
  if (!dir.exists()) {
    if (!QDir().mkpath(GetIntelligentFilePath())) {
      qDebug() << "mkpath() failed:" << GetIntelligentFilePath();
      return std::make_shared<ActStatusInternalError>("Database");
    }
  }

  // Check job can start
  QString project_name;
  act_status = this->CheckWSJobCanStart(ActProjectStatusEnum::kIntelligentDownloadSending, project_id, project_name);
  if (!IsActStatusSuccess(act_status)) {  // thread can't start
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return act_status;
  }

  // Create a std::promise object
  std::shared_ptr<std::promise<void>> signal_sender = std::make_shared<std::promise<void>>();

  // Fetch std::future object associated with promise
  std::future<void> signal_receiver = signal_sender->get_future();

  qDebug() << project_name << "Spawn intelligent questionnaire download thread...";
  std::shared_ptr<std::thread> thread_ptr;
  thread_ptr =
      std::make_shared<std::thread>(&act::core::ActCore::StartIntelligentQuestionnaireDownloadThread, this,
                                    std::cref(ws_listener_id), std::move(signal_receiver), project_id, download);

  // #ifdef _WIN32
  //   // Set the thread name
  //   std::wstring thread_name = L"StartIntelligentRequestThread";
  //   HRESULT hr = SetThreadDescription(thread_ptr->native_handle(), thread_name.c_str());
  //   if (FAILED(hr)) {
  //     // Handle error
  //   }
  // #endif

  // Insert thread handler to pools
  qDebug() << project_name << "Insert thread handler to pools...";
  ws_thread_handler_pools.insert(project_id, make_pair(signal_sender, thread_ptr));

  return act_status;
}

void ActCore::StartIntelligentQuestionnaireDownloadThread(const qint64 &ws_listener_id,
                                                          std::future<void> signal_receiver, qint64 project_id,
                                                          ActIntelligentQuestionnaireDownload download) {
  ACT_STATUS_INIT();

  // Waiting for caller thread
  std::this_thread::yield();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  }

  // Check license
  /* if (!this->GetLicense().GetFeature().GetIntelligent()) {
    QString error_msg = QString("The License not support Intelligent.");
    qCritical() << __func__ << error_msg.toStdString().c_str();
    act_status = std::make_shared<ActBadRequest>(error_msg);
    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());
    return;
  } */

  // Get AI server URL(domain & port)
  ActSystem sys = this->GetSystemConfig();
  QString ai_assistant_local_endpoint = sys.GetIntelligentLocalEndpoint();
  QString http_proxy_endpoint = GetHttpProxyEndpointFromEnviron();

  download.SetProjectId(QString::number(project_id));

  download.SetIntelligentEndpoint(this->GetSystemConfig().GetIntelligentEndpoint());

  download.SetSessionId(sys.GetSerialNumber());

  download.SetSysVersion(sys.GetActVersion());

  // TODO: Temporary backend handling, remove once frontend is implemented
  download.SetProfile(project.GetProfile());
  download.SetProjectMode(project.GetProjectMode());
  download.SetQuestionnaireMode(project.GetQuestionnaireMode());

  // Start MultipleTransaction
  act::core::g_core.EnableMultipleTransaction(project_id);

  // Start send user input to Moxa AI server
  act::intelligent::ActIntelligent intelligent;
  act::intelligent::ActAIServerConnectConfiguration server_connect_cfg(ai_assistant_local_endpoint,
                                                                       http_proxy_endpoint);
  act_status = intelligent.StartIntelligentQuestionnaireDownload(download, server_connect_cfg);
  if (!IsActStatusRunning(act_status)) {
    qCritical() << project.GetProjectName() << "Start Intelligent questionnaire download failed";

    std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
        ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload, *act_status);
    this->SendMessageToWSListener(ws_listener_id, ws_resp->ToString(ws_resp->key_order_).toStdString().c_str());

    // Disable MultipleTransaction & Stop Transaction
    act::core::g_core.DisableMultipleTransaction(project_id);
    act::core::g_core.StopTransaction(project_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kIntelligentDownloadSending;

  // Reference: https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
  while (signal_receiver.wait_for(std::chrono::milliseconds(200)) == std::future_status::timeout) {
    act_status = intelligent.GetStatus();

    // Dequeue
    while (!intelligent.response_queue_.isEmpty()) {
      ActIntelligentResponse intelligent_response = intelligent.response_queue_.dequeue();
      ActIntelligentResponseWSResponse ws_resp(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload,
                                               ActStatusType::kRunning, intelligent_response);
      qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
      this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
    }

    if (act_status->GetStatus() != ActStatusType::kRunning) {
      if (act_status->GetStatus() != ActStatusType::kFinished) {
        // error
        std::shared_ptr<ActBaseErrorMessageResponse> ws_resp =
            ActWSResponseErrorTransfer(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload, *act_status);
        qDebug() << ws_resp->ToString(ws_resp->key_order_).toStdString().c_str();
        this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);
      }

      break;
    }
  }

  qInfo() << project.GetProjectName() << "Thread is going to close";

  // Disable MultipleTransaction
  act::core::g_core.DisableMultipleTransaction(project_id);

  // [bugfix:2141] call stop need to abort immediately
  if (act_status->GetStatus() == ActStatusType::kRunning) {
    intelligent.Stop();
    qCritical() << project.GetProjectName() << "Abort download questionnaire thread";

    this->project_status_list[project_id] = ActProjectStatusEnum::kAborted;

    // Stop Transaction
    act::core::g_core.StopTransaction(project_id);

    return;
  }

  this->project_status_list[project_id] = ActProjectStatusEnum::kFinished;
  // Commit Transaction
  act::core::g_core.CommitTransaction(project_id);

  // Send finished status reply to client
  ActBaseResponse ws_resp(ActWSCommandEnum::kStartIntelligentQuestionnaireDownload, ActStatusType::kFinished);
  qDebug() << __func__ << ws_resp.ToString(ws_resp.key_order_).toStdString().c_str();
  this->SendMessageToListener(ActWSTypeEnum::kSpecified, false, ws_resp, ws_listener_id);

  return;
}

}  // namespace core
}  // namespace act
