
#include "act_mqtt_client.hpp"

#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  // AF node
  ActMqttClient mqtt_client;
  qDebug() << "create a mqtt client";

  // mqtt_client.messageHandler = [](const QString &topic, const ActMqttMessage &message) {
  //   qDebug() << "Received message from topic:" << topic << "with content:" << message.ToString();
  // };

  QObject::connect(&mqtt_client, &ActMqttClient::messageReceived,
                   [&mqtt_client](const QString &topic, const ActMqttMessage &message) {
                     qDebug() << "Received message from topic:" << topic
                              << "with content:" << message.ToString().toStdString().c_str();
                   });

  QObject::connect(&mqtt_client, &ActMqttClient::connectionStatusChanged, [&mqtt_client](bool connected) {
    if (connected) {
      qDebug() << "Connected to the MQTT broker (MQTT 3.1.1).";
      // Here to set what topics you want to subscribe
      mqtt_client.subscribeToTopic(ActMqttEventTopicEnum::kPortLinkDown);
      mqtt_client.subscribeToTopic(ActMqttEventTopicEnum::kPortLinkUp);
      mqtt_client.subscribeToTopic(ActMqttEventTopicEnum::kLoginFail);
      qDebug() << "subscribe topics";
    }
  });

  mqtt_client.connectToBroker();

  return app.exec();
}