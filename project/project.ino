#include "secrets.h"

#include <ESP8266WiFi.h>
#include <MQTT.h> // https://github.com/256dpi/arduino-mqtt

WiFiClient net;
MQTTClient mqttClient;

bool pressed = false;
bool released = true;

void connect()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start MQTT client
  mqttClient.begin(MQTT_HOST, net);

  Serial.println("Trying to connect to MQTT broker...");
  while (!mqttClient.connect("doorbell"))
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Connected to MQTT broker.");
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  // Wifi client mode
  String ssid = WIFI_SSID;
  String password = WIFI_PASSWORD;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  connect();

  Serial.print("Connected to ");
  Serial.println(ssid);

  unsigned long bootTime = millis();
  Serial.println("Booted after " + String(bootTime / 1000.0) + " seconds");
}

void loop()
{
  delay(10);
  mqttClient.loop();

  if (!mqttClient.connected())
  {
    connect();
  }

  const bool wasPressed = pressed;
  const bool wasReleased = released;

  // Read the adc sensor value
  int sensorValue = analogRead(A0);

  if (true)
  {
    Serial.println(sensorValue);
  }

  // Analog read goes from 0 - 1023
  int threshold = 10;

  // Update pressed and released states
  if (sensorValue < threshold)
  {
    pressed = false;
    released = true;
  }
  else
  {
    pressed = true;
    released = false;
  }

  if (pressed && !wasPressed)
  {
    Serial.println("Press Detected");
    mqttClient.publish("doorbell/pressed", "pressed");
  }

  if (released && !wasReleased)
  {
    Serial.println("Release Detected");
    mqttClient.publish("doorbell/released", "released");
  }
}
