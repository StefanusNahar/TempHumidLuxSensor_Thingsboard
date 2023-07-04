#include <Arduino.h>
#include <Wire.h>
#include <DHTesp.h>
#include <ThingsBoard.h>
#include <WiFi.h>
#include "BH1750.h"

#define THINGSBOARD_SERVER  "demo.thingsboard.io"
#define WIFI_SSID "Nahar_deco"
#define WIFI_PASSWORD "Nahar1407"

#define DHT_PIN 15
#define BH1750_ADDRESS 0x23  // Address of the light sensor
#define SDA_PIN 21            // SDA pin - I2C
#define SCL_PIN 22            // SCL pin - I2C

#define THINGSBOARD_ACCESS_TOKEN "lP1eh47h7uhqXqdlNgrg"

WiFiClient espClient;
ThingsBoard tb(espClient);
DHTesp dht;
BH1750 lightMeter;

void WifiConnect();
void SensorData();

void WifiConnect()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}

void SensorData()
{
  digitalWrite(LED_BUILTIN, HIGH);
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float lux = lightMeter.readLightLevel();

    if (dht.getStatus() == DHTesp::ERROR_NONE) {
      Serial.printf("Temperature: %.2f \u00B0C, Humidity: %.2f %%, light: %.2f lux\n",
      temperature, humidity, lux);   
      if (tb.connected())
        {
          tb.sendTelemetryFloat("temperature", temperature);
          tb.sendTelemetryFloat("humidity", humidity);
          tb.sendTelemetryFloat("light", lux);
        }
      }
    else {
    Serial.printf("error");
    }
}

bool connectToThingsBoard() {
    return tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN);
  }
bool shouldSendSensorData() {
  return (millis() % 3000 == 0) && tb.connected();
}

void setup()
{
  Serial.begin(115200);
  dht.setup(DHT_PIN, DHTesp::DHT11);
  Wire.begin(SDA_PIN, SCL_PIN);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);

  WifiConnect();

  if (connectToThingsBoard()) {
    Serial.println("Connected to ThingsBoard");
  } else {
    Serial.println("Error connecting to ThingsBoard");
  }
  Serial.println("System ready.");
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  tb.loop();
  if(shouldSendSensorData()) {
    SensorData();
  }
}
