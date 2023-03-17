#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h> //make sure adress in Adafruit_BMP280.h is 0x76
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

#include "esp_sleep.h"

char pubtopic[64];
float bmp_offset = 1.0;  //hPa offset for newer BMP280

#include "commons.h"

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;  // I2
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial) delay(100);  // wait for native usb

  if (!bmp.begin()) {
    Serial.println(F("Could not find BMP280!"));
  }

  if (!aht.begin()) {
    Serial.println(F("Could not find AHTxx!"));
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  setupWifi();
  setupOTA();

  client.setServer(mqtt_server, 1883);

  sensorData();

  ArduinoOTA.handle();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  //go to sleep
  Serial.println("Going to sleep. Sleep time: " + String(TIME_TO_SLEEP) + " seconds");
  esp_deep_sleep_start();
}

void setupWifi() {
#if defined(IP_STATIC)
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
#endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting for Wi-Fi...");
  }
  Serial.print("Wi-Fi connected! IP: ");
  Serial.println(WiFi.localIP());
}

void setupOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.setHostname(ota_name);
  ArduinoOTA.begin();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_topic)) {
      Serial.println("connected");
      // Subscribe
      // strcpy(pubtopic, mqtt_topic);
      // strcat(pubtopic, "/output");
      // client.subscribe(pubtopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sensorData() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Temperature in Celsius
  float bmptemp = bmp.readTemperature();
  float bmppressure = bmp.readPressure();
  bmppressure = (bmppressure / 100) - bmp_offset;

  sensors_event_t ahthum, ahttemp;
  aht.getEvent(&ahthum, &ahttemp);  // populate temp and humidity objects with fresh data

  float avgtemp = (bmptemp + (float)ahttemp.temperature) / (float)2;

  char avgtempString[8];
  dtostrf(avgtemp, 1, 2, avgtempString);
  strcpy(pubtopic, mqtt_topic);
  strcat(pubtopic, "/temperature");
  client.publish(pubtopic, avgtempString);

  char presString[8];
  dtostrf(bmppressure, 4, 2, presString);
  strcpy(pubtopic, mqtt_topic);
  strcat(pubtopic, "/pressure");
  client.publish(pubtopic, presString);

  char humString[8];
  dtostrf(ahthum.relative_humidity, 1, 0, humString);
  strcpy(pubtopic, mqtt_topic);
  strcat(pubtopic, "/humidity");
  client.publish(pubtopic, humString);
  client.flush();

  Serial.print("BMP Air temperature: ");
  Serial.println(bmptemp);
  Serial.print("BMP Air pressure: ");
  Serial.println(bmppressure);

  Serial.print("AHT Temperature: ");
  Serial.println(ahttemp.temperature);
  Serial.print("AHT Humidity: ");
  Serial.println(ahthum.relative_humidity);
}

void loop() {
}