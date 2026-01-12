/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#include <QQueue>

#include "act_intelligent_request.hpp"
#include "act_json.hpp"
#include "act_moxa_ai_client.hpp"
#include "act_moxa_ai_client_agent.hpp"
#include "act_status.hpp"

#pragma once

// #define ACT_RESTFUL_CLIENT_HTTP_PORT 80  /// < The restful client
namespace act {
namespace intelligent {

class ActAIServerConnectConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, base_url, Baseurl);  ///< The Restful Base URL
  ACT_JSON_FIELD(QString, proxy, Proxy);       ///< The Restful Proxy

 public:
  /**
   * @brief Construct a new Act A I Server Connect Configuration object
   *
   */
  ActAIServerConnectConfiguration() {
    base_url_ = "https://127.0.0.1:59000/";
    proxy_ = "";
  };

  /**
   * @brief Construct a new Act A I Server Connect Configuration object
   *
   * @param base_url
   * @param proxy
   */
  ActAIServerConnectConfiguration(const QString &base_url, const QString &proxy) : ActAIServerConnectConfiguration() {
    base_url_ = base_url;
    proxy_ = proxy;
  };
};

class ActIntelligent {
  Q_GADGET

  ACT_JSON_FIELD(bool, stop_flag, StopFlag);   ///< StopFlag item
  ACT_JSON_FIELD(quint8, progress, Progress);  ///< Progress item

  // for thread
  ACT_STATUS intelligent_act_status_;                ///< Intelligent thread status
  std::unique_ptr<std::thread> intelligent_thread_;  ///< Intelligent thread item

 public:
  QQueue<ActIntelligentResponse> response_queue_;

  /**
   * @brief Construct a new Act Restful Client Handler object
   *
   */
  ActIntelligent();

  /**
   * @brief Destroy the Act Intelligent object
   *
   */
  ~ActIntelligent();

  /**
   * @brief Stop the Intelligent thread
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Stop();

  /**
   * @brief Get the status of the Intelligent thread object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetStatus();

  /**
   * @brief  Update progress
   *
   * @param progress
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateProgress(quint8 progress);

  /**
   * @brief Triggered IntelligentRequest Sender for thread
   *
   * @param project
   * @param check_end_station
   * @param mapping_result
   */
  void TriggerIntelligentRequestSenderForThread(const ActIntelligentRecognizeRequest &recognize_request,
                                                const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Intelligent Request Sender
   *
   * @param recognize_request
   * @param server_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS IntelligentRequestSender(const ActIntelligentRecognizeRequest &recognize_request,
                                      const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Start Intelligent request
   *
   * @param recognize_request
   * @param server_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentRequest(const ActIntelligentRecognizeRequest &recognize_request,
                                     const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Triggered IntelligentUpload Sender for thread
   *
   * @param project
   * @param check_end_station
   * @param mapping_result
   */
  void TriggerIntelligentQuestionnaireUploadThread(const ActIntelligentQuestionnaireUpload &upload,
                                                   const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Intelligent Upload Sender
   *
   * @param upload
   * @param server_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS IntelligentQuestionnaireUpload(const ActIntelligentQuestionnaireUpload &upload,
                                            const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Start Intelligent Upload (Verified) Questionnaire
   *
   * @param upload
   * @param server_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentQuestionnaireUpload(const ActIntelligentQuestionnaireUpload &upload,
                                                 const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Triggered IntelligentQuestionnaireDownload Sender for thread
   *
   * @param server_connect_cfg
   */
  void TriggerIntelligentQuestionnaireDownloadThread(const ActIntelligentQuestionnaireDownload &download,
                                                     const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Intelligent Questionnaire Download Sender
   *
   * @param server_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS IntelligentQuestionnaireDownloadSender(const ActIntelligentQuestionnaireDownload &download,
                                                    const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Start Intelligent Questionnaire Download
   *
   * @param server_connect_cfg
   * @return ACT_STATUS
   */
  ACT_STATUS StartIntelligentQuestionnaireDownload(const ActIntelligentQuestionnaireDownload &download,
                                                   const ActAIServerConnectConfiguration &server_connect_cfg);

  /**
   * @brief Send request to Moxa AI server
   *
   * @param recognize_request
   * @param base_url
   * @param proxy
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS SendRequestToMoxaAIServer(const ActIntelligentRecognizeRequest &recognize_request, const QString &base_url,
                                       const QString &proxy, ActIntelligentResponse &response);

  /**
   * @brief Send report to Moxa AI server
   *
   * @param report
   * @param base_url
   * @param proxy
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS SendReportToMoxaAIServer(const ActIntelligentReport &report, const QString &base_url, const QString &proxy,
                                      ActIntelligentResponse &response);

  /**
   * @brief Get the History From Moxa AI Server
   *
   * @param report
   * @param base_url
   * @param proxy
   * @param response
   * @return ACT_STATUS
   */
  ACT_STATUS GetHistoryFromMoxaAIServer(const ActIntelligentHistory &report, const QString &base_url,
                                        const QString &proxy, ActIntelligentHistoryResponse &response);
};
}  // namespace intelligent

}  // namespace act
