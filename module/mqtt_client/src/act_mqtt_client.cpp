#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

#include "act_mqtt_client.hpp"

const QString ActMqttClient::AF_BROKER_IP = "127.0.0.1";
const quint16 ActMqttClient::AF_BROKER_PORT = 59001;
const QString ActMqttClient::AF_TOPIC_PREFIX = "events/log/";

ActMqttClient::ActMqttClient(QObject *parent) : QObject(parent) {
  client = new QMqttClient(this);

  connect(client, &QMqttClient::messageReceived, this, &ActMqttClient::onMessageReceived);
  connect(client, &QMqttClient::stateChanged, this, &ActMqttClient::onStateChanged);
}

ActMqttClient::~ActMqttClient() {
  disconnectFromBroker();
  delete client;
}

void ActMqttClient::connectToBroker() {
  // kene+
  /*
  client->setHostname(AF_BROKER_IP);
  */
  client->setHostname(GetBrokerAddress());
  /*
  client->setPort(AF_BROKER_PORT);
  */
  client->setPort(GetBrokerPort());
  // kene-
  client->setProtocolVersion(QMqttClient::ProtocolVersion::MQTT_3_1_1);
  client->connectToHost();
}

void ActMqttClient::disconnectFromBroker() { client->disconnectFromHost(); }

void ActMqttClient::subscribeToTopic(const ActMqttEventTopicEnum topic) {
  // turn topic from int to QString
  QString topic_ = QString::number(static_cast<int>(topic));
  QString fullTopic = AF_TOPIC_PREFIX + topic_;

  auto subscription = client->subscribe(fullTopic);
  if (!subscription) {
    qCritical() << "Failed to subscribe to topic:" << fullTopic;
    return;
  }

  qDebug() << "Successfully subscribed to topic:" << fullTopic;
}

void ActMqttClient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic) {
  // QByteArray -> QString -> object
  QString message_str = QString::fromUtf8(message);
  ActMqttMessage mqtt_message;
  mqtt_message.FromString(message_str);

  qDebug() << "Received message from topic:" << topic.name() << "with content:" << message_str.toStdString().c_str();

  // execute user defined message handler function
  // messageHandler(topic.name(), mqtt_message);

  emit messageReceived(topic.name(), mqtt_message);
}

void ActMqttClient::onStateChanged(QMqttClient::ClientState state) {
  switch (state) {
    case QMqttClient::Connecting:
      qDebug() << "Connecting to broker";
      break;
    case QMqttClient::Connected:
      qDebug() << "Connected to broker.";
      emit connectionStatusChanged(true);
      break;
    case QMqttClient::Disconnected:
      qDebug() << "Disconnected from broker.";
      emit connectionStatusChanged(false);
      break;
    default:
      break;
  }
}
