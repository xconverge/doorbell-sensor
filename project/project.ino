#include "secrets.h"

#include <EmonLib.h>
#include <ESP8266WiFi.h>
#include <MQTT.h> // https://github.com/256dpi/arduino-mqtt

EnergyMonitor energyMonitor;
WiFiClient net;
MQTTClient mqttClient;

bool pressed = false;
bool released = true;

float Icalibration = 30.50;

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

  energyMonitor.current(A0, Icalibration);
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
  // Calculate Irms only
  double Irms = energyMonitor.calcIrms(1480);

  // Debug print
  if (false)
  {
    Serial.println(Irms);
  }

  // Tested empirically
  const double threshold = 0.07;
  const bool wasPressed = pressed;
  const bool wasReleased = released;

  // Update pressed and released states
  if (Irms < threshold)
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

    mqttClient.loop();
    if (!mqttClient.connected())
    {
      connect();
    }

    Serial.println("Sending pressed message");
    mqttClient.publish("doorbell/pressed", "ON");
    delay(10);
  }
}
