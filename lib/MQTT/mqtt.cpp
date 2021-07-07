#include "mqtt.h"
#include <string>

#define MQTT_PORT (1884)

void callback(char* topic, byte* message, unsigned int length){
Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(2, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(2, LOW);
    }
  }
}

void mqtt::init(){
    client.setServer(mqttServer, MQTT_PORT);
    client.setCallback(callback);
}

void mqtt::reconnect(){
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP8266Client")) {
        Serial.println("connected");
        client.subscribe("esp32/output"); 
        } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
        }
    }
}

void mqtt::publishBME280stat(float temperature, float pressure, float humidity){
  char sendMsg[10] = {0};
  snprintf(sendMsg, sizeof(sendMsg), "%f", temperature);
  client.publish("esp32/temperature", sendMsg);

  memset(sendMsg, 0, sizeof(sendMsg));
  snprintf(sendMsg, sizeof(sendMsg), "%f", pressure);
  client.publish("esp32/pressure", sendMsg);

  memset(sendMsg, 0, sizeof(sendMsg));
  snprintf(sendMsg, sizeof(sendMsg), "%f", humidity);
  client.publish("esp32/humidity", sendMsg);
}