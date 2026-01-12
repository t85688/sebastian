
#pragma once

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSslError>
#include <QTimer>
#include <QWebSocket>

#include "act_json.hpp"

class WsClient : public QObject {
  Q_OBJECT
 public:
  explicit WsClient(const QUrl &url, QObject *parent = nullptr) : QObject(parent), m_url(std::move(url)) {}

 public slots:
  void open() {
    m_ws = new QWebSocket();
    m_ws->setParent(this);
    m_pingTimer = new QTimer(this);

    connect(m_ws, &QWebSocket::connected, this, &WsClient::onConnected);
    connect(m_ws, &QWebSocket::disconnected, this, &WsClient::onDisconnected);
    connect(m_ws, &QWebSocket::textMessageReceived, this, &WsClient::onText);
    connect(m_ws, &QWebSocket::sslErrors, this, &WsClient::onSslErrors);
    connect(m_ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WsClient::onError);

    connect(m_pingTimer, &QTimer::timeout, this, [this] { m_ws->ping(); });
    m_pingTimer->start(20000);

    qInfo() << "[WS] open ->" << m_url;
    m_ws->open(m_url);
  }

  void close() {
    if (m_ws && m_ws->state() == QAbstractSocket::ConnectedState) {
      qInfo() << "[WS] close";
      m_ws->close(QWebSocketProtocol::CloseCodeNormal, "client close");
    }
  }

  void sendRequest(QString msg) {
    if (m_ws->state() != QAbstractSocket::ConnectedState) {
      qInfo() << "[WS] not connected, open ->" << m_url;
      m_ws->open(m_url);
      m_pending = std::move(msg);
      return;
    }
    qInfo() << "[WS] send request:" << msg.toStdString().c_str();
    m_ws->sendTextMessage(std::move(msg));
  }

 signals:
  void errorOccurred(const QString &message);
  void receivedMessage(const QString &message);

 private slots:
  void onConnected() {
    qInfo() << "[WS] connected";
    m_pingTimer->start(20000);
    if (!m_pending.isEmpty()) {  // 若先收到 sendRequest，再補送
      m_ws->sendTextMessage(m_pending);
      m_pending.clear();
    }
  }

  void onDisconnected() {
    qInfo() << "[WS] disconnected";
    m_pingTimer->stop();
  }

  void onError(QAbstractSocket::SocketError) {
    qInfo() << "[WS] error:" << m_ws->errorString();
    emit errorOccurred(m_ws->errorString());
  }

  void onSslErrors(const QList<QSslError> &errors) {
    qInfo() << "[WS] ignore SSL errors";
    m_ws->ignoreSslErrors();  // ← 忽略 TLS 憑證錯誤
  }

  void onText(const QString &msg) {
    qInfo() << "[WS] recv:" << msg.toStdString().c_str();
    emit receivedMessage(msg);
  }

 private:
  QWebSocket *m_ws = nullptr;
  QUrl m_url;
  QTimer *m_pingTimer = nullptr;
  QString m_pending;
};

class WsBaseCommand : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, op_code, OpCode);
  ACT_JSON_FIELD(qint64, project_id, ProjectId);

 public:
  WsBaseCommand() {
    SetOpCode(static_cast<qint64>(ActWSCommandEnum::kTestStart));
    SetProjectId(-1);
  }

  explicit WsBaseCommand(ActWSCommandEnum op_code, qint64 project_id) {
    SetOpCode(static_cast<qint64>(op_code));
    SetProjectId(project_id);
  }
};

class DeployWsCommand : public WsBaseCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, design_baseline_id, DesignBaselineId);
  ACT_JSON_COLLECTION(QList, qint64, id, Id);
  ACT_JSON_FIELD(bool, skip_mapping_device, SkipMappingDevice);

 public:
  DeployWsCommand() : WsBaseCommand() {
    SetDesignBaselineId(-1);
    SetSkipMappingDevice(false);
  }

  DeployWsCommand(ActWSCommandEnum op_code, qint64 project_id, qint64 design_baseline_id, const QList<qint64> &id,
                  bool skip_mapping_device)
      : WsBaseCommand(op_code, project_id) {
    SetDesignBaselineId(design_baseline_id);
    SetId(id);
    SetSkipMappingDevice(skip_mapping_device);
  }
};

class ScanTopologyWsCommand : public WsBaseCommand {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, new_topology, NewTopology);

 public:
  ScanTopologyWsCommand() : WsBaseCommand() { SetNewTopology(false); }

  ScanTopologyWsCommand(ActWSCommandEnum op_code, qint64 project_id, bool new_topology)
      : WsBaseCommand(op_code, project_id) {
    SetNewTopology(new_topology);
  }
};

class WsBaseResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, op_code, OpCode);               ///< The opcode in the websocket
  ACT_JSON_FIELD(qint64, status_code, StatusCode);       ///< The status type of this websocket
  ACT_JSON_FIELD(QString, error_message, ErrorMessage);  ///< The error message of this return entry

 public:
  WsBaseResponse() {
    this->op_code_ = static_cast<qint64>(ActWSCommandEnum::kTestStart);
    this->status_code_ = static_cast<qint64>(ActStatusType::kSuccess);
  }

  WsBaseResponse(const qint64 &op_code, const qint64 &status_code) {
    this->op_code_ = op_code;
    this->status_code_ = status_code;
  }
};

class DeployWsResponseData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(quint8, progress, Progress);            ///< Progress item
  ACT_JSON_ENUM(ActStatusType, status, Status);          ///< The status type of this device
  ACT_JSON_FIELD(QString, error_message, ErrorMessage);  ///< The error message of this return entry

 public:
  DeployWsResponseData() {
    this->id_ = -1;
    this->progress_ = 0;
    this->status_ = ActStatusType::kRunning;
    this->error_message_ = "";
  }
};

class DeployWsResponse : public WsBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(DeployWsResponseData, data, Data);

 public:
  DeployWsResponse() {}
  explicit DeployWsResponse(qint64 op_code, qint64 status_type, const DeployWsResponseData &data)
      : WsBaseResponse(op_code, status_type), data_(data) {}
};

class ScanResultData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, ip, Ip);
  ACT_JSON_FIELD(QString, mac_address, MacAddress);
  ACT_JSON_FIELD(QString, model_name, ModelName);
};

class ScanTopologyWsResponseData : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);  ///< Progress item
  ACT_JSON_COLLECTION_OBJECTS(QList, ScanResultData, scan_result, ScanResult);

 public:
  ScanTopologyWsResponseData() { this->progress_ = 0; }
};

class ScanTopologyWsResponse : public WsBaseResponse {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ScanTopologyWsResponseData, data, Data);

 public:
  ScanTopologyWsResponse() {}
  explicit ScanTopologyWsResponse(qint64 op_code, qint64 status_type, const ScanTopologyWsResponseData &data)
      : WsBaseResponse(op_code, status_type), data_(data) {}
};