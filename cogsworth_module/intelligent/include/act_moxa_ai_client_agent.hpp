#pragma once

#include <QDebug>
#include <QQueue>
#include <QString>

#include "act_device.hpp"
#include "act_intelligent_request.hpp"
#include "act_json.hpp"
#include "act_moxa_ai_client.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "oatpp-curl/ProxyRequestExecutor.hpp"
#include "oatpp/core/data/resource/InMemoryData.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/web/mime/multipart/PartList.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/web/protocol/http/outgoing/MultipartBody.hpp"

#define ACT_RESTFUL_CLIENT_REQUEST_SUCCESS (200)  /// < The restful request status success  code

// class TimedDataStreamProvider : public oatpp::data::stream::ReadCallback {
//  private:
//   int m_counter;
//   std::shared_ptr<oatpp::data::stream::InputStream> m_inputStream;
//   v_io_size m_bufferSize;
//   oatpp::String m_buffer;

//  public:
//   TimedDataStreamProvider() : m_counter(0) {}

//   oatpp::v_io_size read(void *buffer, v_buff_size count, oatpp::async::Action &action) override {
//     if (m_counter < 10) {
//       // Wait for one second
//       std::this_thread::sleep_for(std::chrono::seconds(2));
//       // Generate data
//       std::string data = "Data chunk " + std::to_string(m_counter++) + "\n";
//       v_buff_size size = std::min<v_buff_size>(count, data.size());
//       std::memcpy(buffer, data.data(), size);

//       return size;
//     } else {
//       return 0;  // No more data to send
//     }
//   }
// };

/**
 * @brief The Moxa get string request type enum class
 *
 */
enum class ActMoxaAIRequestTypeEnum { kPostTheRecognizeRequest };

/**
 * @brief The QMap Moxa get string request type enum mapping
 *
 */
static const QMap<QString, ActMoxaAIRequestTypeEnum> kActMoxaAIRequestTypeEnumMap = {
    {"PostTheRecognizeRequest", ActMoxaAIRequestTypeEnum::kPostTheRecognizeRequest}};

/**
 * @brief The MoxaAI Client agent class
 *
 */
class ActMoxaAIClientAgent {
 private:
  QString token_;
  QString base_url_;
  QString proxy_;
  std::shared_ptr<ActMoxaAIClient> client_;

  /**
   * @brief Create a Restful Client object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CreateRestfulClient() {
    ACT_STATUS_INIT();

    try {
      // Create ObjectMapper for serialization of DTOs
      auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      // Create RequestExecutor which will execute ApiClient's requests
      std::string baseUrl = base_url_.toStdString();
      std::string proxyUrl = proxy_.toStdString();
      std::shared_ptr<oatpp::web::client::RequestExecutor> requestExecutor =
          oatpp::curl::ProxyRequestExecutor::createShared(baseUrl, proxyUrl);
      client_ = ActMoxaAIClient::createShared(requestExecutor, objectMapper);
    } catch (std::exception &e) {
      qCritical() << __func__ << "Create the Act Moxa AI client failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
    return act_status;
  }

 public:
  /**
   * @brief Construct a new ActMoxaAIClientAgent object
   *
   * @param device_ip
   * @param port
   */
  ActMoxaAIClientAgent(const QString &base_url, const QString &proxy) {
    base_url_ = base_url;
    proxy_ = proxy;
  }

  /**
   * @brief Init ActMoxaAIClientAgent object
   *
   * @return ACT_STATUS
   */
  ACT_STATUS Init() {
    ACT_STATUS_INIT();
    act_status = CreateRestfulClient();
    if (!IsActStatusSuccess(act_status)) {
      return act_status;
    }
    return act_status;
  }

  /**
   * @brief Do Post Request by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS PostTheRecognizeRequestAndUseStreamingReceive(const ActIntelligentRecognizeRequest &recognize_request,
                                                           QQueue<ActIntelligentResponse> &response_queue) {
    ACT_STATUS_INIT();

    qDebug() << __func__ << QString("recognize_request: %1").arg(recognize_request.ToString()).toStdString().c_str();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      oatpp::web::mime::multipart::Headers part_headers;
      auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();

      // intelligentEndpoint
      auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part1);
      part1->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("intelligentEndpoint") + "\"");
      part1->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetIntelligentEndpoint()).toStdString())));

      // sessionId
      auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part2);
      part2->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sessionId") + "\"");
      part2->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetSessionId()).toStdString())));

      // projectId
      auto part3 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part3);
      part3->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectId") + "\"");
      part3->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetProjectId()).toStdString())));

      // text
      auto part4 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part4);
      part4->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("text") + "\"");
      part4->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetText()).toStdString())));

      // sysVersion
      auto part5 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part5);
      part5->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sysVersion") + "\"");
      part5->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetSysVersion()).toStdString())));

      // autoSend
      auto part6 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part6);
      part6->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("autoSend") + "\"");
      part6->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(recognize_request.GetAutoSend() ? "true" : "false")));

      // projectMode
      auto part7 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part7);
      part7->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectMode") + "\"");
      part7->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(oatpp::String(
          QString(GetStringFromEnum<ActProjectModeEnum>(recognize_request.GetProjectMode(), kActProjectModeEnumMap))
              .toStdString())));

      // questionnaireMode
      auto part8 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part8);
      part8->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("questionnaireMode") + "\"");
      part8->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActQuestionnaireModeEnum>(recognize_request.GetQuestionnaireMode(),
                                                                            kActQuestionnaireModeEnumMap))
                            .toStdString())));

      // profile
      auto part9 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part9);
      part9->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("profile") + "\"");
      part9->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActServiceProfileForLicenseEnum>(recognize_request.GetProfile(),
                                                                                   kActServiceProfileForLicenseEnumMap))
                            .toStdString())));

      auto multiple_part_body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

      response = client_->DoPostTheRecognizeRequestByStreamAPI(multiple_part_body);
      auto response_status_code = response->getStatusCode();
      // QString response_status_descr = QString(response->getStatusDescription());
      QString response_status_descr = QString::fromUtf8(response->getStatusDescription()->c_str());

      // AI test
      // Check response status code
      if (response_status_code == 200) {
        // // Read and print the streaming response
        // auto body_stream = response->getBodyStream();

        // oatpp::String buffer(1024);
        // oatpp::async::Action action;
        // oatpp::v_io_size bytes_read;

        // // bytes_read == 0: to indicate end-of-file.
        // while ((bytes_read = body_stream->read(buffer->data(), buffer->size(), action)) > 0) {
        //   QString data = QString::fromUtf8(buffer->data(), bytes_read);
        //   qDebug() << QString("Response(size:%1):").arg(bytes_read) << data.toStdString().c_str();

        //   // Find the positions of the first '{' and the last '}' to skip noise data
        //   int startPos = data.indexOf('{');
        //   int endPos = data.lastIndexOf('}');

        //   if (startPos != -1 && endPos != -1 && startPos < endPos) {
        //     // Extract the substring from startPos to endPos (inclusive)
        //     ActIntelligentResponse intelligent_response;
        //     auto reply = data.mid(startPos, endPos - startPos + 1);
        //     intelligent_response.FromString(reply);

        //     response_queue.enqueue(intelligent_response);
        //   }
        // }

        oatpp::async::Action action;
        auto stream = response->getBodyStream();
        oatpp::String buffer;
        oatpp::v_io_size bytes_read;

        while (true) {
          // Read chunk size (trailing '\r\n')
          QString chunk_size_str;
          while (true) {
            char ch;
            bytes_read = stream->read(&ch, 1, action);
            if (bytes_read <= 0 || ch == '\r') {  // trailing '\r'
              break;
            }
            chunk_size_str.append(ch);
          }
          stream->read(&chunk_size_str[0], 1, action);  // Read '\n'

          // Convert chunk size from hex to integer using QString
          bool ok;
          int chunk_size = chunk_size_str.toInt(&ok, 16);
          if (!ok || chunk_size == 0) {  // last chunk or conversion error
            break;
          }

          // Allocate buffer for the chunk data
          buffer = oatpp::String((v_buff_size)chunk_size);

          // Read chunk data
          int total_bytes_read = 0;
          while (total_bytes_read < chunk_size) {
            // Calculate the number of bytes to read
            int to_read = qMin(chunk_size - total_bytes_read, (int)buffer->size());
            bytes_read = stream->read(buffer->data() + total_bytes_read, to_read, action);
            if (bytes_read <= 0) {  // end
              break;
            }
            total_bytes_read += bytes_read;
          }

          // Output the chunk data
          QString data = QString::fromUtf8(buffer->data(), total_bytes_read);
          qDebug() << QString("Response(%1):%2").arg(total_bytes_read).arg(data);
          ActIntelligentResponse intelligent_response;
          intelligent_response.FromString(data);
          response_queue.enqueue(intelligent_response);

          // Read trailing '\r\n'
          stream->read(&chunk_size_str[0], 2, action);
        }

      } else {
        QString response_body = response->readBodyToString()->c_str();
        qDebug() << __func__
                 << QString("Response(PostTheRecognizeRequestAndUseStreamingReceive)(%1):%2")
                        .arg(response_status_code)
                        .arg(response_body)
                        .toStdString()
                        .c_str();
        ActIntelligentResponse intelligent_response;
        intelligent_response.Getstatus().Setcode(response_status_code);
        intelligent_response.Getstatus().Setmessage(response_status_descr);

        response_queue.enqueue(intelligent_response);
      }

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  /**
   * @brief Do Post upload by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS PostTheQuestionnaireUpload(const ActIntelligentQuestionnaireUpload &upload,
                                        QQueue<ActIntelligentResponse> &response_queue) {
    ACT_STATUS_INIT();

    // qDebug() << __func__ << QString("upload: %1").arg(upload.ToString()).toStdString().c_str();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      oatpp::web::mime::multipart::Headers part_headers;
      auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();

      // intelligentEndpoint
      auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part1);
      part1->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("intelligentEndpoint") + "\"");
      part1->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(upload.GetIntelligentEndpoint()).toStdString())));

      // sessionId
      auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part2);
      part2->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sessionId") + "\"");
      part2->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(upload.GetSessionId()).toStdString())));

      // projectId
      auto part3 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part3);
      part3->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectId") + "\"");
      part3->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(upload.GetProjectId()).toStdString())));

      // sysVersion
      auto part4 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part4);
      part4->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sysVersion") + "\"");
      part4->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(upload.GetSysVersion()).toStdString())));

      // fileName
      auto part5 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part5);
      part5->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("fileName") + "\"");
      part5->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(upload.GetFileName()).toStdString())));

      // file
      auto part6 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part6);
      part6->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("file") + "\"");
      part6->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(upload.GetFile()).toStdString())));

      // projectMode
      auto part8 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part8);
      part8->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectMode") + "\"");
      part8->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActProjectModeEnum>(upload.GetProjectMode(), kActProjectModeEnumMap))
                            .toStdString())));

      // questionnaireMode
      auto part9 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part9);
      part9->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("questionnaireMode") + "\"");
      part9->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActQuestionnaireModeEnum>(upload.GetQuestionnaireMode(),
                                                                            kActQuestionnaireModeEnumMap))
                            .toStdString())));

      // profile
      auto part10 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part10);
      part10->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("profile") + "\"");
      part10->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActServiceProfileForLicenseEnum>(upload.GetProfile(),
                                                                                   kActServiceProfileForLicenseEnumMap))
                            .toStdString())));

      auto multiple_part_body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

      if (upload.GetVerify()) {
        response = client_->DoPostTheQuestionnaireVerify(multiple_part_body);
      } else {
        response = client_->DoPostQuestionnaireUpload(multiple_part_body);
      }

      auto response_status_code = response->getStatusCode();
      // QString response_status_descr = QString(response->getStatusDescription());
      QString response_status_descr = QString::fromUtf8(response->getStatusDescription()->c_str());

      // AI test
      // Check response status code
      if (response_status_code == 200) {
        // // Read and print the streaming response
        // auto body_stream = response->getBodyStream();

        // oatpp::String buffer(1024);
        // oatpp::async::Action action;
        // oatpp::v_io_size bytes_read;

        // // bytes_read == 0: to indicate end-of-file.
        // while ((bytes_read = body_stream->read(buffer->data(), buffer->size(), action)) > 0) {
        //   QString data = QString::fromUtf8(buffer->data(), bytes_read);
        //   qDebug() << QString("Response(size:%1):").arg(bytes_read) << data.toStdString().c_str();

        //   // Find the positions of the first '{' and the last '}' to skip noise data
        //   int startPos = data.indexOf('{');
        //   int endPos = data.lastIndexOf('}');

        //   if (startPos != -1 && endPos != -1 && startPos < endPos) {
        //     // Extract the substring from startPos to endPos (inclusive)
        //     ActIntelligentResponse intelligent_response;
        //     auto reply = data.mid(startPos, endPos - startPos + 1);
        //     intelligent_response.FromString(reply);

        //     response_queue.enqueue(intelligent_response);
        //   }
        // }

        oatpp::async::Action action;
        auto stream = response->getBodyStream();
        oatpp::String buffer;
        oatpp::v_io_size bytes_read;

        while (true) {
          // Read chunk size (trailing '\r\n')
          QString chunk_size_str;
          while (true) {
            char ch;
            bytes_read = stream->read(&ch, 1, action);
            if (bytes_read <= 0 || ch == '\r') {  // trailing '\r'
              break;
            }
            chunk_size_str.append(ch);
          }
          stream->read(&chunk_size_str[0], 1, action);  // Read '\n'

          // Convert chunk size from hex to integer using QString
          bool ok;
          int chunk_size = chunk_size_str.toInt(&ok, 16);
          if (!ok || chunk_size == 0) {  // last chunk or conversion error
            break;
          }

          // Allocate buffer for the chunk data
          buffer = oatpp::String((v_buff_size)chunk_size);

          // Read chunk data
          int total_bytes_read = 0;
          while (total_bytes_read < chunk_size) {
            // Calculate the number of bytes to read
            int to_read = qMin(chunk_size - total_bytes_read, (int)buffer->size());
            bytes_read = stream->read(buffer->data() + total_bytes_read, to_read, action);
            if (bytes_read <= 0) {  // end
              break;
            }
            total_bytes_read += bytes_read;
          }

          // Output the chunk data
          QString data = QString::fromUtf8(buffer->data(), total_bytes_read);
          qDebug() << QString("Response(%1):%2").arg(total_bytes_read).arg(data);
          ActIntelligentResponse intelligent_response;
          intelligent_response.FromString(data);
          response_queue.enqueue(intelligent_response);

          // Read trailing '\r\n'
          stream->read(&chunk_size_str[0], 2, action);
        }

      } else {
        QString response_body = response->readBodyToString()->c_str();
        qDebug() << __func__
                 << QString("Response(PostTheQuestionnaireUpload)(%1):%2")
                        .arg(response_status_code)
                        .arg(response_body)
                        .toStdString()
                        .c_str();
        ActIntelligentResponse intelligent_response;
        intelligent_response.Getstatus().Setcode(response_status_code);
        intelligent_response.Getstatus().Setmessage(response_status_descr);

        response_queue.enqueue(intelligent_response);
      }

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  /**
   * @brief Get questionnaire template by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS GetQuestionnaireTemplate(const ActIntelligentQuestionnaireDownload &download,
                                      QQueue<ActIntelligentResponse> &response_queue) {
    ACT_STATUS_INIT();

    qDebug() << __func__ << QString("download: %1").arg(download.ToString()).toStdString().c_str();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;

      const std::string intelligentEndpoint = QUrl::toPercentEncoding(download.GetIntelligentEndpoint()).toStdString();
      const std::string sessionId = QUrl::toPercentEncoding(download.GetSessionId()).toStdString();
      const std::string projectId = QUrl::toPercentEncoding(download.GetProjectId()).toStdString();
      const std::string sysVersion = QUrl::toPercentEncoding(download.GetSysVersion()).toStdString();

      QString projectModeQStr =
          GetStringFromEnum<ActProjectModeEnum>(download.GetProjectMode(), kActProjectModeEnumMap);
      const std::string projectMode = QUrl::toPercentEncoding(projectModeQStr).toStdString();

      QString questionnaireModeQStr =
          GetStringFromEnum<ActQuestionnaireModeEnum>(download.GetQuestionnaireMode(), kActQuestionnaireModeEnumMap);
      const std::string questionnaireMode = QUrl::toPercentEncoding(questionnaireModeQStr).toStdString();

      QString profileQStr = GetStringFromEnum<ActServiceProfileForLicenseEnum>(download.GetProfile(),
                                                                               kActServiceProfileForLicenseEnumMap);
      const std::string profile = QUrl::toPercentEncoding(profileQStr).toStdString();

      response = client_->DoGetQuestionnaireTemplate(intelligentEndpoint, sessionId, projectId, sysVersion, projectMode,
                                                     questionnaireMode, profile);

      auto response_status_code = response->getStatusCode();
      QString response_status_descr = QString::fromUtf8(response->getStatusDescription()->c_str());

      auto contentLengthHeader = response->getHeader("Content-Length");
      if (contentLengthHeader) {
        v_int64 contentLength = std::stoll(contentLengthHeader->c_str());
        qDebug() << "FileSize: Content-Length:" << contentLength;
      } else {
        qDebug() << "FileSize: Content-Length header is missing";
      }

      // Check response status code
      if (response_status_code == 200) {
        // Read the response body
        auto body_stream = response->getBodyStream();
        oatpp::data::stream::BufferOutputStream buffer;
        char tempBuffer[4096];
        oatpp::data::stream::transfer(body_stream, &buffer, 0, tempBuffer, sizeof(tempBuffer));
        auto response_body = buffer.toString();  // Save the response body to a .xlsx file
        QString file_name =
            QString("%1/Questionnaire_%2.xlsx").arg(GetIntelligentFilePath()).arg(download.GetProjectId());
        QFile file(file_name);
        if (file.open(QIODevice::WriteOnly)) {
          file.write(response_body->data(), response_body->size());
          file.close();
          qDebug() << "File saved successfully.";
        } else {
          qDebug() << "Failed to open file for writing.";
        }

      } else {
        QString response_body = response->readBodyToString()->c_str();
        qDebug() << __func__
                 << QString("Response(GetQuestionnaireTemplate)(%1):%2")
                        .arg(response_status_code)
                        .arg(response_body)
                        .toStdString()
                        .c_str();
        ActIntelligentResponse intelligent_response;
        intelligent_response.Getstatus().Setcode(response_status_code);
        intelligent_response.Getstatus().Setmessage(response_status_descr);

        response_queue.enqueue(intelligent_response);
      }

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  /**
   * @brief Do Post Request by restful
   *
   * @return ACT_STATUS
   */
  ACT_STATUS PostTheRecognizeRequest(const ActIntelligentRecognizeRequest &recognize_request,
                                     ActIntelligentResponse &result_response) {
    ACT_STATUS_INIT();

    qDebug() << __func__ << QString("recognize_request: %1").arg(recognize_request.ToString()).toStdString().c_str();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      oatpp::web::mime::multipart::Headers part_headers;
      auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();

      // intelligentEndpoint
      auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part1);
      part1->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("intelligentEndpoint") + "\"");
      part1->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetIntelligentEndpoint()).toStdString())));

      // sessionId
      auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part2);
      part2->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sessionId") + "\"");
      part2->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetSessionId()).toStdString())));

      // projectId
      auto part3 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part3);
      part3->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectId") + "\"");
      part3->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetProjectId()).toStdString())));

      // text
      auto part4 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part4);
      part4->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("text") + "\"");
      part4->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetText()).toStdString())));

      // sysVersion
      auto part5 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part5);
      part5->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sysVersion") + "\"");
      part5->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(recognize_request.GetSysVersion()).toStdString())));

      // autoSend
      auto part6 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part6);
      part6->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("autoSend") + "\"");
      part6->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(recognize_request.GetAutoSend() ? "true" : "false")));

      // projectMode
      auto part7 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part7);
      part7->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectMode") + "\"");
      part7->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(oatpp::String(
          QString(GetStringFromEnum<ActProjectModeEnum>(recognize_request.GetProjectMode(), kActProjectModeEnumMap))
              .toStdString())));

      // questionnaireMode
      auto part8 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part8);
      part8->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("questionnaireMode") + "\"");
      part8->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActQuestionnaireModeEnum>(recognize_request.GetQuestionnaireMode(),
                                                                            kActQuestionnaireModeEnumMap))
                            .toStdString())));

      // profile
      auto part9 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part9);
      part9->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("profile") + "\"");
      part9->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActServiceProfileForLicenseEnum>(recognize_request.GetProfile(),
                                                                                   kActServiceProfileForLicenseEnumMap))
                            .toStdString())));

      auto multiple_part_body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

      response = client_->DoPostTheRecognizeRequest(multiple_part_body);
      auto response_status_code = response->getStatusCode();

      // Check success
      // if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) {
      //   qCritical() << __func__ << "Response Status(" << response_status_code
      //               << "):" << response->getStatusDescription()->c_str();
      //   // return std::make_shared<ActStatusInternalError>("RESTful");
      // }

      QString response_body = response->readBodyToString()->c_str();
      // qDebug() << __func__ << QString("Response(%1)").arg(response_status_code).toStdString().c_str();

      result_response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS PostTheRecognizeReport(const ActIntelligentReport &report, ActIntelligentResponse &result_response) {
    ACT_STATUS_INIT();

    qDebug() << __func__ << QString("Report intelligent feedback: %1").arg(report.ToString()).toStdString().c_str();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      oatpp::web::mime::multipart::Headers part_headers;
      auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();

      // intelligentEndpoint
      auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part1);
      part1->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("intelligentEndpoint") + "\"");
      part1->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(report.GetIntelligentEndpoint()).toStdString())));

      // id
      auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part2);
      part2->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("id") + "\"");
      part2->setPayload(
          std::make_shared<oatpp::data::resource::InMemoryData>(oatpp::String(QString(report.GetId()).toStdString())));

      // positive
      auto part3 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part3);
      part3->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("positive") + "\"");
      part3->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(report.GetPositive() ? "true" : "false")));

      auto multiple_part_body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

      response = client_->DoPostTheRecognizeReport(multiple_part_body);
      auto response_status_code = response->getStatusCode();

      // Check success
      // if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) {
      //   qCritical() << __func__ << "Response Status(" << response_status_code
      //               << "):" << response->getStatusDescription()->c_str();
      //   // return std::make_shared<ActStatusInternalError>("RESTful");
      // }

      QString response_body = response->readBodyToString()->c_str();
      // qDebug() << __func__
      //          << QString("Response(DoPostTheRecognizeReport)(%1):%2")
      //                 .arg(response_status_code)
      //                 .arg(response_body)
      //                 .toStdString()
      //                 .c_str();

      result_response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }

  ACT_STATUS GetHistory(const ActIntelligentHistory &history, ActIntelligentHistoryResponse &history_response) {
    ACT_STATUS_INIT();

    qDebug() << __func__ << QString("Get intelligent history: %1").arg(history.ToString()).toStdString().c_str();

    try {
      // Send request
      std::shared_ptr<oatpp::web::protocol::http::incoming::Response> response;
      // QString session_id = "404404";
      // QString project_id = "1002";
      // QString text = "clear";

      oatpp::web::mime::multipart::Headers part_headers;
      auto multipart = oatpp::web::mime::multipart::PartList::createSharedWithRandomBoundary();

      // intelligentEndpoint
      auto part1 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part1);
      part1->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("intelligentEndpoint") + "\"");
      part1->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(history.GetIntelligentEndpoint()).toStdString())));

      // sessionId
      auto part2 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part2);
      part2->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sessionId") + "\"");
      part2->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(history.GetSessionId()).toStdString())));

      // projectId
      auto part3 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part3);
      part3->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectId") + "\"");
      part3->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(history.GetProjectId()).toStdString())));

      // sysVersion
      auto part4 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part4);
      part4->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("sysVersion") + "\"");
      part4->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(history.GetSysVersion()).toStdString())));

      // projectMode
      auto part5 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part5);
      part5->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("projectMode") + "\"");
      part5->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActProjectModeEnum>(history.GetProjectMode(), kActProjectModeEnumMap))
                            .toStdString())));

      // questionnaireMode
      auto part6 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part6);
      part6->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("questionnaireMode") + "\"");
      part6->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActQuestionnaireModeEnum>(history.GetQuestionnaireMode(),
                                                                            kActQuestionnaireModeEnumMap))
                            .toStdString())));

      // profile
      auto part7 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
      multipart->writeNextPartSimple(part7);
      part7->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("profile") + "\"");
      part7->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
          oatpp::String(QString(GetStringFromEnum<ActServiceProfileForLicenseEnum>(history.GetProfile(),
                                                                                   kActServiceProfileForLicenseEnumMap))
                            .toStdString())));

      // offset, not passed when the first loading
      if (!history.GetOffset().isEmpty()) {
        auto part8 = std::make_shared<oatpp::web::mime::multipart::Part>(part_headers);
        multipart->writeNextPartSimple(part8);
        part8->putHeader("Content-Disposition", "form-data; name=\"" + oatpp::String("offset") + "\"");
        part8->setPayload(std::make_shared<oatpp::data::resource::InMemoryData>(
            oatpp::String(QString(history.GetOffset()).toStdString())));
      }

      auto multiple_part_body = std::make_shared<oatpp::web::protocol::http::outgoing::MultipartBody>(multipart);

      response = client_->DoPostTheHistoryRequest(multiple_part_body);
      auto response_status_code = response->getStatusCode();

      // Check success
      // if (response_status_code != ACT_RESTFUL_CLIENT_REQUEST_SUCCESS) {
      //   qCritical() << __func__ << "Response Status(" << response_status_code
      //               << "):" << response->getStatusDescription()->c_str();
      //   // return std::make_shared<ActStatusInternalError>("RESTful");
      // }

      QString response_body = response->readBodyToString()->c_str();
      // qDebug() << __func__
      //          << QString("Response(PostTheHistoryRequest)(%1):%2")
      //                 .arg(response_status_code)
      //                 .arg(response_body)
      //                 .toStdString()
      //                 .c_str();

      history_response.FromString(response_body);

      return act_status;
    } catch (std::exception &e) {
      qCritical() << __func__ << "failed. Error:" << e.what();
      return std::make_shared<ActStatusInternalError>("RESTful");
    }
  }
};
