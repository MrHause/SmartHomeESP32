#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

class mqtt{
        
    private:
        IPAddress mqttServer;
        PubSubClient client;
        WiFiClient espClient;

    public:
        mqtt() : mqttServer(192,168,0,221){
            client.setClient(espClient);
            this->init();
        }

        void init();
        bool isConnected(){ return client.connected(); }
        void reconnect();
        void publishBME280stat(float temperature, float pressure, float humidity);
        void loop(){ client.loop(); }


};

#endif