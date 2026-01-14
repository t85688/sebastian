/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QDebug>
#include <QObject>
#include <QString>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttMessage>

#include "act_json.hpp"

enum class ActMqttEventTopicEnum {
  kReceiveSNMPTrapEvent = 3010001,
  kPortLinkDown = 3010002,
  kPortLinkUp = 3010003,
  kLoginFail = 3010006
};

class ActMqttMessage : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, id);
  ACT_JSON_FIELD(qint64, code, code);
  ACT_JSON_FIELD(QString, severity, severity);
  ACT_JSON_FIELD(QString, source, source);
  ACT_JSON_FIELD(QString, source_IP, sourceIp);
  ACT_JSON_COLLECTION(QList, QString, variables, variables);
  ACT_JSON_FIELD(QString, message, message);
  ACT_JSON_FIELD(QString, timestamp, timestamp);
};

class ActMqttClient : public QObject {
  Q_OBJECT

 public:
  static const QString AF_BROKER_IP;
  static const quint16 AF_BROKER_PORT;
  static const QString AF_TOPIC_PREFIX;

  // std::function<void(const QString &, const ActMqttMessage &)> messageHandler;

  explicit ActMqttClient(QObject *parent = nullptr);
  ~ActMqttClient();

  void connectToBroker();
  void disconnectFromBroker();
  void subscribeToTopic(ActMqttEventTopicEnum topic);

 signals:
  void messageReceived(const QString &topic, const ActMqttMessage &message);
  void connectionStatusChanged(bool connected);

 private slots:
  void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
  void onStateChanged(QMqttClient::ClientState state);

 private:
  QMqttClient *client;
  QString host;
  quint16 port;
};

// kene+
static QString GetBrokerAddress() {
  QString apiAddr = qEnvironmentVariable("MAF_IPC_ADDR", "localhost");
  return apiAddr;
}

static quint16 GetBrokerPort() {
  QString apiPort = qEnvironmentVariable("MAF_IPC_PORT", "59001");
  bool ok;
  quint16 value = apiPort.toUShort(&ok);
  if (ok) {
    return value;
  }
  return 59001;
}
// kene-
