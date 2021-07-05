#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

const char* ssid = "UPC0720490";
const char* password = "x74dbsjnrtnT";
const char* mqtt_server = "192.168.0.221";
IPAddress mqttServer(192,168,0,221);

int LED_BUILTIN = 2;

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint8_t newMesaureTrigger;

void IRAM_ATTR onTimer() {
  newMesaureTrigger = 1;
  Serial.print("hi \n");
}

void callback(char* topic, byte* message, unsigned int length) {
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
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void printValues() {
  Serial.print("Temperature = ");
  float readTemp = bme.readTemperature();
  Serial.print(readTemp);
  Serial.println(" *C");

  char sendMsg[10] = {0};
  snprintf(sendMsg, sizeof(sendMsg), "%f", readTemp);
  client.publish("esp32/temperature", sendMsg);
  
  Serial.print("Pressure = ");
  float readPressure = bme.readPressure() / 100.0F;
  Serial.print(readPressure);
  Serial.println(" hPa");

  memset(sendMsg, 0, sizeof(sendMsg));
  snprintf(sendMsg, sizeof(sendMsg), "%f", readPressure);
  client.publish("esp32/pressure", sendMsg);

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  float readHumidity = bme.readHumidity();
  Serial.print(readHumidity);
  Serial.println(" %");

  
  memset(sendMsg, 0, sizeof(sendMsg));
  snprintf(sendMsg, sizeof(sendMsg), "%f", readHumidity);
  client.publish("esp32/humidity", sendMsg);

  Serial.println();
}

void setup() {
  pinMode (LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, 1884);
  client.setCallback(callback);

  if(!bme.begin(0x76)){
    Serial.print("couldn't connect with bme sensor");
    while (1);
  }else
    Serial.print("Connected with BME 280 ");

  timer = timerBegin(0, 240, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 5000000, true);
  timerAlarmEnable(timer);

  newMesaureTrigger = 0;

}

void loop() {
    if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(newMesaureTrigger){
    newMesaureTrigger = 0;
    printValues();
  }
/*
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
*/
}