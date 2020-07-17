#define MQTT_MAX_PACKET_SIZE 2048 // Maximum packet size (mqtt max = 4kB)
#define MQTT_KEEPALIVE 120        // keepAlive interval in Seconds

#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoLog.h>

#include "config.h"
#include "meter.h"

bool connectedToMQTT = false;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

Smartmeter meter_pv;
Smartmeter meter_load;

void setup_wifi()
{
  delay(10);
  Log.notice("Connecting to %s ", wifi_ssid);

  // delete old config
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Log.notice("WiFi connected - IP address: %s" CR, WiFi.localIP().toString().c_str());
}

void mqttReconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Log.notice("Connecting to MQTT borker on %s : %d ..." CR, mqtt_server, mqtt_port);
    // Attempt to connect
    if (mqttClient.connect(clientId))
    {
      Log.notice("connected" CR);
      // Once connected, publish an announcement...
      const char *topic = "/energy/status";
      char *path = (char *)malloc(1 + strlen(clientId) + strlen(topic));
      strcpy(path, clientId);
      strcat(path, topic);
      mqttClient.publish(path, "online");
    }
    else
    {
      Log.error("failed, rc=%s  try again in 5 seconds" CR, mqttClient.state());
      delay(5000);
    }
  }
}

void setup()
{
  // Setup debug Serial
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_NOTICE, &Serial);
  Log.notice("Hardware serial started" CR);

  // Setup Wifi + MQTT
  setup_wifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttReconnect();

  // Setup Hardware Serial for smart meter of PV
  Serial1.begin(9600, SERIAL_8N1, 19, 21, false);
  meter_pv.setup(&Serial1, &mqttClient, mqtt_topic_pv);
  Log.notice("Hardware serial U1UXD (RX: D19 GPIO19, TX: D21 GPIO21) started at 9600 baudrate." CR);

  // Setup Hardware Serial for smart meter of load
  Serial2.begin(9600, SERIAL_8N1, 16, 17, false);
  meter_load.setup(&Serial2, &mqttClient, mqtt_topic_load);
  Log.notice("Hardware serial U2UXD (RX: RX2 GPIO16, TX: TX2 GPIO17) started at 9600 baudrate." CR);
}

void loop()
{
  unsigned long start = micros();

  // Check and fix Wifi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    setup_wifi();
  }

  // Check and fix MQTT connection
  if (!mqttClient.connected())
  {
    mqttReconnect();
  }
  mqttClient.loop();

  // Process meter readings
  meter_pv.loop();
  meter_load.loop();

  //Log.notice("loop took %d us" CR, micros() - start);
}
